#include "formats_support/gltf/GltfMeshLoader.hpp"

#include "base/diag.hpp"
#include "system/pathname.hpp"

#include "mathex/point3d.hpp"
#include "mathex/quatern.hpp"
#include "mathex/transf3d.hpp"
#include "mathex/vec3.hpp"

#include "utility/string.hpp"

#include "tiny_gltf.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// FileEntry -- opaque cache entry holding the tinygltf model and converted meshes.
// ---------------------------------------------------------------------------

struct GltfMeshLoader::FileEntry
{
    std::unique_ptr<tinygltf::Model> model;
    SysPathName pathName;
};

// ---------------------------------------------------------------------------
// tinygltf accessor helpers (local to this TU)
// ---------------------------------------------------------------------------

template <typename T>
static std::vector<T> readAccessor(const tinygltf::Model& model, int accessorIndex)
{
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size()))
        return {};

    const tinygltf::Accessor& acc = model.accessors[accessorIndex];
    const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = model.buffers[bv.buffer];

    size_t byteOffset = bv.byteOffset + acc.byteOffset;
    size_t stride = bv.byteStride;

    std::vector<T> result(acc.count);

    if (stride == 0 || stride == sizeof(T))
    {
        std::memcpy(result.data(), buf.data.data() + byteOffset, acc.count * sizeof(T));
    }
    else
    {
        for (size_t i = 0; i < acc.count; ++i)
            std::memcpy(&result[i], buf.data.data() + byteOffset + i * stride, sizeof(T));
    }

    return result;
}

static std::vector<uint32_t> readIndices(const tinygltf::Model& model, int accessorIndex)
{
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size()))
        return {};

    const tinygltf::Accessor& acc = model.accessors[accessorIndex];
    const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = model.buffers[bv.buffer];

    size_t byteOffset = bv.byteOffset + acc.byteOffset;
    size_t stride = bv.byteStride;

    std::vector<uint32_t> result(acc.count);

    if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
    {
        size_t s = stride ? stride : sizeof(uint32_t);
        for (size_t i = 0; i < acc.count; ++i)
        {
            uint32_t val = 0;
            std::memcpy(&val, buf.data.data() + byteOffset + i * s, sizeof(uint32_t));
            result[i] = val;
        }
    }
    else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        size_t s = stride ? stride : sizeof(uint16_t);
        for (size_t i = 0; i < acc.count; ++i)
        {
            uint16_t val = 0;
            std::memcpy(&val, buf.data.data() + byteOffset + i * s, sizeof(uint16_t));
            result[i] = static_cast<uint32_t>(val);
        }
    }
    else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
    {
        size_t s = stride ? stride : sizeof(uint8_t);
        for (size_t i = 0; i < acc.count; ++i)
        {
            result[i] = static_cast<uint32_t>(buf.data[byteOffset + i * s]);
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// tinygltf extras helpers
// ---------------------------------------------------------------------------

static double gltfExtraDouble(const tinygltf::Value& extras, const std::string& key, double def = 0.0)
{
    if (extras.Has(key))
    {
        const tinygltf::Value& v = extras.Get(key);
        if (v.IsNumber())
            return v.GetNumberAsDouble();
    }
    return def;
}

static bool gltfExtraBool(const tinygltf::Value& extras, const std::string& key, bool def = true)
{
    if (extras.Has(key))
    {
        const tinygltf::Value& v = extras.Get(key);
        if (v.IsBool())
            return v.Get<bool>();
    }
    return def;
}

// ---------------------------------------------------------------------------
// Convert a tinygltf mesh into the format-agnostic RenI::MeshData.
// ---------------------------------------------------------------------------

static RenI::MeshData convertMesh(const tinygltf::Model& model, int meshIndex)
{
    RenI::MeshData data;

    const tinygltf::Mesh& gltfMesh = model.meshes[meshIndex];
    data.name = gltfMesh.name;

    struct Vec3 { float x, y, z; };
    struct Vec2 { float u, v; };

    for (const tinygltf::Primitive& prim : gltfMesh.primitives)
    {
        auto posIt = prim.attributes.find("POSITION");
        if (posIt == prim.attributes.end())
            continue;

        std::vector<Vec3> positions = readAccessor<Vec3>(model, posIt->second);
        size_t primVerts = positions.size();
        if (primVerts == 0)
            continue;

        std::vector<Vec3> normals;
        auto normIt = prim.attributes.find("NORMAL");
        if (normIt != prim.attributes.end())
            normals = readAccessor<Vec3>(model, normIt->second);

        std::vector<Vec2> texcoords;
        auto uvIt = prim.attributes.find("TEXCOORD_0");
        if (uvIt != prim.attributes.end())
            texcoords = readAccessor<Vec2>(model, uvIt->second);

        RenI::MeshPrimitive mp;

        // Vertices -- glTF is Y-up right-handed; engine is Z-up left-handed.
        // Conversion: engine(x,y,z) = glTF(x,z,y)
        mp.vertices.resize(primVerts);
        for (size_t i = 0; i < primVerts; ++i)
        {
            auto& v = mp.vertices[i];
            v.px = positions[i].x;
            v.py = positions[i].z;
            v.pz = positions[i].y;
            if (i < normals.size())
            {
                v.nx = normals[i].x;
                v.ny = normals[i].z;
                v.nz = normals[i].y;
            }
            if (i < texcoords.size())
            {
                v.tu = texcoords[i].u;
                v.tv = texcoords[i].v;
            }
        }

        // Indices -- reverse winding (swap indices 1<->2 per triangle).
        // The exporter flipped winding for glTF CCW convention; undo it here
        // so the Y->Z axis swap produces correct engine winding.
        mp.indices = readIndices(model, prim.indices);
        for (size_t i = 0; i + 2 < mp.indices.size(); i += 3)
            std::swap(mp.indices[i + 1], mp.indices[i + 2]);

        // Material -> RenI::MeshMaterial
        if (prim.material >= 0 && prim.material < static_cast<int>(model.materials.size()))
        {
            const tinygltf::Material& gltfMat = model.materials[prim.material];
            const auto& pbr = gltfMat.pbrMetallicRoughness;
            const tinygltf::Value& extras = gltfMat.extras;

            RenI::MeshMaterial mm;
            mm.diffuseR = static_cast<float>(pbr.baseColorFactor[0]);
            mm.diffuseG = static_cast<float>(pbr.baseColorFactor[1]);
            mm.diffuseB = static_cast<float>(pbr.baseColorFactor[2]);
            mm.diffuseA = static_cast<float>(pbr.baseColorFactor[3]);

            if (extras.Has("emissiveFactor"))
                mm.emissiveFactor = static_cast<float>(gltfExtraDouble(extras, "emissiveFactor"));
            else
            {
                // Recover from standard glTF emissiveFactor (max channel)
                float ef = 0.0f;
                for (double v : gltfMat.emissiveFactor)
                    ef = std::max(ef, static_cast<float>(v));
                mm.emissiveFactor = ef;
            }

            int sm = static_cast<int>(gltfExtraDouble(extras, "sortMode"));
            if (sm == 1)
                mm.sortMode = RenI::SortMode::InterMeshCoplanar;
            else if (sm == 2)
                mm.sortMode = RenI::SortMode::IntraMeshAlpha;
            else if (sm == 3)
                mm.sortMode = RenI::SortMode::AbsoluteAlphaNegative;
            else if (sm == 4)
                mm.sortMode = RenI::SortMode::AbsoluteAlphaPositive;
            mm.sortPriority = static_cast<short>(gltfExtraDouble(extras, "sortPriority"));

            int sa = static_cast<int>(gltfExtraDouble(extras, "spinAxis"));
            if (sa == 1)
                mm.spinAxis = RenI::SpinAxis::X;
            else if (sa == 2)
                mm.spinAxis = RenI::SpinAxis::Y;
            else if (sa == 3)
                mm.spinAxis = RenI::SpinAxis::Z;

            if (extras.Has("backfaceCull"))
                mm.backfaceCull = gltfExtraBool(extras, "backfaceCull");
            else
                mm.backfaceCull = !gltfMat.doubleSided;

            // Resolve texture name from the best available source.
            // tinygltf mangles image.name/uri during .glb writing, so we
            // try multiple sources in priority order.  The engine's
            // findTextureFile() handles .png->.bmp fallback at load time,
            // so we just need a leaf filename with an extension.
            if (pbr.baseColorTexture.index >= 0
                && pbr.baseColorTexture.index < static_cast<int>(model.textures.size()))
            {
                const tinygltf::Texture& tex = model.textures[pbr.baseColorTexture.index];

                // Collect candidates in priority order
                std::string candidates[3];
                int nCandidates = 0;
                if (!tex.name.empty())
                    candidates[nCandidates++] = tex.name;
                if (tex.source >= 0
                    && tex.source < static_cast<int>(model.images.size()))
                {
                    const tinygltf::Image& img = model.images[tex.source];
                    if (!img.name.empty())
                        candidates[nCandidates++] = img.name;
                    if (!img.uri.empty())
                        candidates[nCandidates++] = img.uri;
                }

                // Pick the first candidate that has an extension
                std::string name;
                for (int ci = 0; ci < nCandidates && name.empty(); ++ci)
                {
                    std::string& c = candidates[ci];
                    // Strip directory prefix
                    auto slash = c.rfind('/');
                    if (slash != std::string::npos)
                        c = c.substr(slash + 1);
                    if (!c.empty() && c.rfind('.') != std::string::npos)
                        name = std::move(c);
                }
                // If nothing had an extension, use the first non-empty
                // candidate and assume .bmp
                if (name.empty())
                {
                    for (int ci = 0; ci < nCandidates; ++ci)
                    {
                        if (!candidates[ci].empty())
                        {
                            name = candidates[ci] + ".bmp";
                            break;
                        }
                    }
                }
                mm.textureName = std::move(name);
            }

            if (mm.emissiveFactor > 0.03f)
            {
                mm.emissiveR = mm.diffuseR * mm.emissiveFactor;
                mm.emissiveG = mm.diffuseG * mm.emissiveFactor;
                mm.emissiveB = mm.diffuseB * mm.emissiveFactor;
            }

            mp.materialIndex = static_cast<int>(data.materials.size());
            data.materials.push_back(std::move(mm));
        }

        data.primitives.push_back(std::move(mp));
    }

    return data;
}

// ---------------------------------------------------------------------------
// Populate HierarchyNode transform from a glTF node's matrix or TRS properties.
// glTF is Y-up right-handed; engine is Z-up left-handed.
// Conversion: M_engine = S * M_gltf * S  where S swaps Y<->Z.
// ---------------------------------------------------------------------------

static void nodeTransform(const tinygltf::Node& node, RenI::HierarchyNode& out)
{
    MexTransform3d t;

    if (node.matrix.size() == 16)
    {
        // glTF column-major: columns are [X_basis, Y_basis, Z_basis, position].
        // After S*M*S: engine X = (gX.x, gX.z, gX.y),
        //              engine Y = (gZ.x, gZ.z, gZ.y),  (glTF Z -> engine Y)
        //              engine Z = (gY.x, gY.z, gY.y),  (glTF Y -> engine Z)
        //              pos      = (p.x,  p.z,  p.y)
        const auto& m = node.matrix;
        MexVec3 xBasis(m[0], m[2], m[1]);
        MexVec3 yBasis(m[8], m[10], m[9]);
        MexVec3 zBasis(m[4], m[6], m[5]);
        MexPoint3d position(m[12], m[14], m[13]);
        t = MexTransform3d(xBasis, yBasis, zBasis, position);
    }
    else
    {
        MexPoint3d position(0, 0, 0);
        if (node.translation.size() == 3)
            position = MexPoint3d(node.translation[0], node.translation[2], node.translation[1]);

        MexQuaternion quat;
        if (node.rotation.size() == 4)
            quat = MexQuaternion(MexVec3(node.rotation[0], node.rotation[2], node.rotation[1]), node.rotation[3]);

        t.position(position);
        t.rotation(quat);
    }

    MexVec3 bx, by, bz;
    t.xBasis(&bx);
    t.yBasis(&by);
    t.zBasis(&bz);
    MexPoint3d pos = t.position();

    out.xBasisX = bx.x(); out.xBasisY = bx.y(); out.xBasisZ = bx.z();
    out.yBasisX = by.x(); out.yBasisY = by.y(); out.yBasisZ = by.z();
    out.zBasisX = bz.x(); out.zBasisY = bz.y(); out.zBasisZ = bz.z();
    out.posX = pos.x(); out.posY = pos.y(); out.posZ = pos.z();
}

// ---------------------------------------------------------------------------
// GltfMeshLoader implementation
// ---------------------------------------------------------------------------

GltfMeshLoader::GltfMeshLoader() = default;

GltfMeshLoader::~GltfMeshLoader()
{
    deleteAll();
}

std::vector<std::string> GltfMeshLoader::supportedExtensions() const
{
    return {"glb", "gltf"};
}

void GltfMeshLoader::deleteAll()
{
    files_.clear();
}

RenI::MeshData GltfMeshLoader::loadMesh(
    const SysPathName& pathName,
    const std::string& meshName)
{
    auto fileIt = files_.find(pathName.pathname());
    FileEntry* entry = nullptr;

    if (fileIt != files_.end())
        entry = fileIt->second.get();
    else
        entry = loadFile(pathName);

    if (!entry || !entry->model)
        return {};

    std::string lcName(meshName);
    Utils::toLowerInPlace(&lcName);

    tinygltf::Model& model = *entry->model;

    int sceneIdx = model.defaultScene >= 0 ? model.defaultScene : 0;
    if (sceneIdx < static_cast<int>(model.scenes.size()))
    {
        // Search nodes for matching combined name or raw mesh name.
        std::function<int(int)> findMesh = [&](int nodeIndex) -> int
        {
            if (nodeIndex < 0 || nodeIndex >= static_cast<int>(model.nodes.size()))
                return -1;

            const tinygltf::Node& node = model.nodes[nodeIndex];
            if (node.mesh >= 0 && node.mesh < static_cast<int>(model.meshes.size()))
            {
                std::string mName = model.meshes[node.mesh].name;
                if (mName.empty())
                    mName = node.name;

                std::string combinedName = node.name + 'X' + mName;
                std::string lcCombined(combinedName);
                Utils::toLowerInPlace(&lcCombined);
                if (lcCombined == lcName)
                    return node.mesh;

                std::string lcMeshName(mName);
                Utils::toLowerInPlace(&lcMeshName);
                if (lcMeshName == lcName)
                    return node.mesh;
            }

            for (int childIdx : node.children)
            {
                int result = findMesh(childIdx);
                if (result >= 0)
                    return result;
            }
            return -1;
        };

        for (int nodeIdx : model.scenes[sceneIdx].nodes)
        {
            int meshIdx = findMesh(nodeIdx);
            if (meshIdx >= 0)
                return convertMesh(model, meshIdx);
        }
    }

    return {};
}

RenI::HierarchyData GltfMeshLoader::loadHierarchy(const SysPathName& pathName)
{
    RenI::HierarchyData result;

    auto fileIt = files_.find(pathName.pathname());
    if (fileIt == files_.end())
        loadFile(pathName);

    fileIt = files_.find(pathName.pathname());
    if (fileIt == files_.end())
        return result;

    FileEntry& entry = *fileIt->second;
    tinygltf::Model& model = *entry.model;

    int sceneIdx = model.defaultScene >= 0 ? model.defaultScene : 0;
    if (sceneIdx < static_cast<int>(model.scenes.size()))
    {
        std::function<RenI::HierarchyNode(int)> buildNode = [&](int nodeIndex) -> RenI::HierarchyNode
        {
            RenI::HierarchyNode hnode;

            if (nodeIndex < 0 || nodeIndex >= static_cast<int>(model.nodes.size()))
                return hnode;

            const tinygltf::Node& node = model.nodes[nodeIndex];
            nodeTransform(node, hnode);
            hnode.instanceName = node.name;
            hnode.filePath = pathName.pathname();
            hnode.scale = 1.0;

            bool hasMesh = node.mesh >= 0 && node.mesh < static_cast<int>(model.meshes.size());
            if (hasMesh)
            {
                std::string meshName = model.meshes[node.mesh].name;
                if (meshName.empty())
                    meshName = node.name;
                hnode.meshName = node.name + 'X' + meshName;
            }

            for (int childIdx : node.children)
                hnode.children.push_back(buildNode(childIdx));

            return hnode;
        };

        for (int nodeIdx : model.scenes[sceneIdx].nodes)
            result.roots.push_back(buildNode(nodeIdx));
    }

    return result;
}

GltfMeshLoader::FileEntry* GltfMeshLoader::loadFile(const SysPathName& pathName)
{
    auto entry = std::make_unique<FileEntry>();
    entry->pathName = pathName;
    entry->model = std::make_unique<tinygltf::Model>();

    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ok = false;
    std::string ext = pathName.extension();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "glb")
        ok = loader.LoadBinaryFromFile(entry->model.get(), &err, &warn, pathName.pathname());
    else
        ok = loader.LoadASCIIFromFile(entry->model.get(), &err, &warn, pathName.pathname());

    if (!warn.empty())
        std::cout << "[GLTF] Warning: " << warn << std::endl;

    if (!ok)
    {
        std::cerr << "[GLTF] Failed to load " << pathName << ": " << err << std::endl;
        return nullptr;
    }

    // No pre-conversion -- meshes are converted on-the-fly in loadMesh().

    FileEntry* raw = entry.get();
    files_[pathName.pathname()] = std::move(entry);
    return raw;
}

