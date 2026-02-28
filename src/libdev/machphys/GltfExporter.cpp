#include "machphys/ModelExporter.hpp"

#include "formats_support/MeshData.hpp"

#include "mathex/transf3d.hpp"
#include "mathex/vec3.hpp"

#include "render/mesh.hpp"
#include "render/meshinst.hpp"

#include "ctl/vector.hpp"

#include "world4d/Entity/Composite.hpp"
#include "world4d/Entity/Entity.hpp"
#include "world4d/Entity/Link.hpp"

#include "system/pathname.hpp"

#include "tiny_gltf.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// Game-specific material extras keys.
// These encode the repurposed .x material fields so that a round-trip
// glTF -> game loader can reconstruct RenMaterial flags exactly.
static constexpr const char* kExtraSortMode = "sortMode";
static constexpr const char* kExtraSortPriority = "sortPriority";
static constexpr const char* kExtraEmissiveFactor = "emissiveFactor";
static constexpr const char* kExtraSpinAxis = "spinAxis";
static constexpr const char* kExtraBackfaceCull = "backfaceCull";

// Add a buffer view + accessor to the glTF model.
// Returns the accessor index.
static int addAccessor(
    tinygltf::Model& model,
    int bufferIndex,
    const void* data,
    size_t byteLength,
    int componentType,
    int type,
    size_t count,
    std::vector<unsigned char>& bufferData)
{
    size_t offset = bufferData.size();

    // Align to 4 bytes
    while (offset % 4 != 0)
    {
        bufferData.push_back(0);
        offset = bufferData.size();
    }

    const auto* bytes = static_cast<const unsigned char*>(data);
    bufferData.insert(bufferData.end(), bytes, bytes + byteLength);

    tinygltf::BufferView bv;
    bv.buffer = bufferIndex;
    bv.byteOffset = offset;
    bv.byteLength = byteLength;

    if (type == TINYGLTF_TYPE_SCALAR && componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
        bv.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    else
        bv.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    int bvIndex = static_cast<int>(model.bufferViews.size());
    model.bufferViews.push_back(bv);

    tinygltf::Accessor acc;
    acc.bufferView = bvIndex;
    acc.byteOffset = 0;
    acc.componentType = componentType;
    acc.type = type;
    acc.count = count;

    // Compute min/max for positions
    if (type == TINYGLTF_TYPE_VEC3 && componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
    {
        const float* fdata = static_cast<const float*>(data);
        if (count > 0)
        {
            acc.minValues = {fdata[0], fdata[1], fdata[2]};
            acc.maxValues = {fdata[0], fdata[1], fdata[2]};
            for (size_t i = 1; i < count; ++i)
            {
                for (int c = 0; c < 3; ++c)
                {
                    acc.minValues[c] = std::min(acc.minValues[c], static_cast<double>(fdata[i * 3 + c]));
                    acc.maxValues[c] = std::max(acc.maxValues[c], static_cast<double>(fdata[i * 3 + c]));
                }
            }
        }
    }

    int accIndex = static_cast<int>(model.accessors.size());
    model.accessors.push_back(acc);
    return accIndex;
}

// Build a glTF material from a MeshMaterial and add it to the model.
// Returns the material index.  textureMap deduplicates images by name.
static int addGltfMaterial(
    tinygltf::Model& model,
    const RenI::MeshMaterial& md,
    std::map<std::string, int>& textureMap)
{
    tinygltf::Material mat;
    mat.pbrMetallicRoughness.baseColorFactor = {md.diffuseR, md.diffuseG, md.diffuseB, md.diffuseA};
    mat.pbrMetallicRoughness.metallicFactor = 0.0;
    mat.pbrMetallicRoughness.roughnessFactor = 1.0;

    if (md.emissiveFactor > 0.01f)
        mat.emissiveFactor = {md.diffuseR * md.emissiveFactor, md.diffuseG * md.emissiveFactor, md.diffuseB * md.emissiveFactor};

    if (md.diffuseA < 0.99f)
    {
        mat.alphaMode = "BLEND";
        mat.alphaCutoff = 0.0;
    }

    mat.doubleSided = !md.backfaceCull;

    // Game-specific extras
    mat.extras = tinygltf::Value(tinygltf::Value::Object({
        {kExtraSortMode, tinygltf::Value(static_cast<int>(md.sortMode))},
        {kExtraSortPriority, tinygltf::Value(static_cast<int>(md.sortPriority))},
        {kExtraEmissiveFactor, tinygltf::Value(static_cast<double>(md.emissiveFactor))},
        {kExtraSpinAxis, tinygltf::Value(static_cast<int>(md.spinAxis))},
        {kExtraBackfaceCull, tinygltf::Value(md.backfaceCull)},
    }));

    // Texture reference via standard glTF image->texture->baseColorTexture.
    // The game loader resolves the texture by the image name/uri at load time.
    if (!md.textureName.empty())
    {
        auto [it, inserted] = textureMap.try_emplace(md.textureName, -1);
        if (inserted)
        {
            // Build a glTF-compliant URI: leaf filename with .png extension.
            // glTF only supports PNG and JPEG; the game textures are BMP.
            std::string leafName = md.textureName;
            auto slash = leafName.rfind('/');
            if (slash != std::string::npos)
                leafName = leafName.substr(slash + 1);
            auto dot = leafName.rfind('.');
            if (dot != std::string::npos)
                leafName = leafName.substr(0, dot);
            leafName += ".png";

            tinygltf::Image image;
            image.uri = leafName;
            image.name = md.textureName; // full engine path for our loader
            int imgIdx = static_cast<int>(model.images.size());
            model.images.push_back(std::move(image));

            tinygltf::Texture tex;
            tex.source = imgIdx;
            tex.name = md.textureName;
            int texIdx = static_cast<int>(model.textures.size());
            model.textures.push_back(std::move(tex));

            it->second = texIdx;
        }
        mat.pbrMetallicRoughness.baseColorTexture.index = it->second;
        mat.pbrMetallicRoughness.baseColorTexture.texCoord = 0;
    }

    int idx = static_cast<int>(model.materials.size());
    model.materials.push_back(std::move(mat));
    return idx;
}

// Convert a MeshData into a glTF mesh and add it to the model.
// Returns the mesh index.
static int addGltfMesh(
    tinygltf::Model& model,
    int bufferIndex,
    std::vector<unsigned char>& bufferData,
    const RenI::MeshData& meshData,
    std::vector<int>& materialMap)
{
    tinygltf::Mesh gltfMesh;
    gltfMesh.name = meshData.name;

    for (const auto& prim : meshData.primitives)
    {
        size_t vertCount = prim.vertices.size();
        if (vertCount == 0)
            continue;

        // Deinterleave MeshVertex into separate position/normal/texcoord arrays.
        std::vector<float> positions(vertCount * 3);
        std::vector<float> normals(vertCount * 3);
        std::vector<float> texcoords(vertCount * 2);

        // Engine is Z-up left-handed; glTF is Y-up right-handed.
        // Conversion: glTF(x,y,z) = engine(x,z,y)
        for (size_t i = 0; i < vertCount; ++i)
        {
            const auto& v = prim.vertices[i];
            positions[i * 3]     = v.px;
            positions[i * 3 + 1] = v.pz;
            positions[i * 3 + 2] = v.py;
            normals[i * 3]       = v.nx;
            normals[i * 3 + 1]   = v.nz;
            normals[i * 3 + 2]   = v.ny;
            texcoords[i * 2]     = v.tu;
            texcoords[i * 2 + 1] = v.tv;
        }

        tinygltf::Primitive gp;
        gp.mode = TINYGLTF_MODE_TRIANGLES;

        // Positions
        int posAcc = addAccessor(
            model, bufferIndex,
            positions.data(), positions.size() * sizeof(float),
            TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3,
            vertCount, bufferData);
        gp.attributes["POSITION"] = posAcc;

        // Normals
        int normAcc = addAccessor(
            model, bufferIndex,
            normals.data(), normals.size() * sizeof(float),
            TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3,
            vertCount, bufferData);
        gp.attributes["NORMAL"] = normAcc;

        // Texcoords
        int uvAcc = addAccessor(
            model, bufferIndex,
            texcoords.data(), texcoords.size() * sizeof(float),
            TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2,
            vertCount, bufferData);
        gp.attributes["TEXCOORD_0"] = uvAcc;

        // Indices -- reverse winding (swap indices 1<->2 per triangle).
        // The engine uses CCW front-face in Z-up LH space; the Y<->Z axis swap
        // (det=-1) flips winding to CW, but glTF expects CCW.
        std::vector<uint32_t> flippedIndices(prim.indices);
        for (size_t i = 0; i + 2 < flippedIndices.size(); i += 3)
            std::swap(flippedIndices[i + 1], flippedIndices[i + 2]);

        int idxAcc = addAccessor(
            model, bufferIndex,
            flippedIndices.data(), flippedIndices.size() * sizeof(uint32_t),
            TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_SCALAR,
            flippedIndices.size(), bufferData);
        gp.indices = idxAcc;

        // Material
        if (prim.materialIndex >= 0)
            gp.material = materialMap[prim.materialIndex];

        gltfMesh.primitives.push_back(std::move(gp));
    }

    int idx = static_cast<int>(model.meshes.size());
    model.meshes.push_back(std::move(gltfMesh));
    return idx;
}

// Check if a LOD level has any meshes across composite + links
static bool gltfLodHasAnyMesh(const W4dComposite& composite, W4dLOD lod)
{
    if (composite.hasMesh(lod))
        return true;
    for (const W4dLink* link : composite.links())
    {
        if (link && link->hasMesh(lod))
            return true;
    }
    return false;
}

// Convert a MexTransform3d to a column-major 4x4 float array for glTF node.matrix.
// Engine is Z-up left-handed; glTF is Y-up right-handed.
// Conversion: M_gltf = S * M_engine * S  where S swaps Y<->Z.
static std::vector<double> transformToMatrix(const MexTransform3d& t)
{
    MexVec3 xb, yb, zb;
    t.xBasis(&xb);
    t.yBasis(&yb);
    t.zBasis(&zb);
    MexPoint3d pos = t.position();

    // glTF uses column-major order.
    // After the Y<->Z swap: new X=(Xx,Xz,Xy), new Y=(Zx,Zz,Zy), new Z=(Yx,Yz,Yy)
    return {
        xb.x(), xb.z(), xb.y(), 0.0,
        zb.x(), zb.z(), zb.y(), 0.0,
        yb.x(), yb.z(), yb.y(), 0.0,
        pos.x(), pos.z(), pos.y(), 1.0,
    };
}

std::string MachPhysModelExporter::writeGltfFile(
    const W4dComposite& composite,
    const SysPathName& outputDir,
    const std::string& baseName,
    W4dLOD lod)
{
    if (!gltfLodHasAnyMesh(composite, lod))
        return {};

    std::string glbFileName = baseName + ".glb";

    std::string fullPath = outputDir.pathname() + "/" + glbFileName;

    tinygltf::Model model;
    model.asset.version = "2.0";
    model.asset.generator = "Machines glTF Exporter";

    // Single buffer for all data
    tinygltf::Buffer buffer;
    std::vector<unsigned char> bufferData;
    bufferData.reserve(64 * 1024);
    int bufferIndex = 0;
    model.buffers.push_back(buffer); // placeholder, will set data at end

    // Texture deduplication map (textureName -> glTF texture index)
    std::map<std::string, int> textureMap;

    // Build parent->children map from all links
    const auto& links = composite.links();
    std::map<const W4dEntity*, std::vector<const W4dLink*>> linkTree;
    for (const W4dLink* link : links)
    {
        if (!link)
            continue;
        const W4dEntity* parent = link->hasParent() ? link->pParent() : nullptr;
        linkTree[parent].push_back(link);
    }

    // Helper to add a mesh for an entity at the given LOD
    auto addEntityMesh = [&](const W4dEntity& entity) -> int
    {
        if (!entity.hasMesh(lod))
            return -1;

        const RenMeshInstance& meshInst = entity.mesh(lod);
        RenI::MeshData meshData = RenMesh::extractMeshData(meshInst);
        if (meshData.primitives.empty())
            return -1;

        // Add materials first
        std::vector<int> materialMap;
        for (const auto& md : meshData.materials)
        {
            int gltfMatIdx = addGltfMaterial(model, md, textureMap);
            materialMap.push_back(gltfMatIdx);
        }

        return addGltfMesh(model, bufferIndex, bufferData, meshData, materialMap);
    };

    // Recursive lambda to create nodes for links
    std::function<int(const W4dLink&)> addLinkNode;
    addLinkNode = [&](const W4dLink& link) -> int
    {
        tinygltf::Node node;
        node.name = link.name().empty() ? "link" : link.name();
        node.matrix = transformToMatrix(link.localTransform());

        int meshIdx = addEntityMesh(link);
        if (meshIdx >= 0)
            node.mesh = meshIdx;

        int nodeIdx = static_cast<int>(model.nodes.size());
        model.nodes.push_back(std::move(node));

        // Add children
        auto childIt = linkTree.find(&link);
        if (childIt != linkTree.end())
        {
            for (const W4dLink* childLink : childIt->second)
            {
                int childNodeIdx = addLinkNode(*childLink);
                model.nodes[nodeIdx].children.push_back(childNodeIdx);
            }
        }

        return nodeIdx;
    };

    // Root node
    tinygltf::Node rootNode;
    const std::string& rootName = composite.name();
    rootNode.name = rootName.empty() ? baseName : rootName;
    rootNode.matrix = transformToMatrix(composite.localTransform());

    int rootMeshIdx = addEntityMesh(composite);
    if (rootMeshIdx >= 0)
        rootNode.mesh = rootMeshIdx;

    int rootNodeIdx = static_cast<int>(model.nodes.size());
    model.nodes.push_back(std::move(rootNode));

    // Add direct children of the composite
    auto rootChildIt = linkTree.find(&composite);
    if (rootChildIt != linkTree.end())
    {
        for (const W4dLink* childLink : rootChildIt->second)
        {
            int childNodeIdx = addLinkNode(*childLink);
            model.nodes[rootNodeIdx].children.push_back(childNodeIdx);
        }
    }

    // Scene
    tinygltf::Scene scene;
    scene.name = baseName;
    scene.nodes.push_back(rootNodeIdx);
    model.scenes.push_back(scene);
    model.defaultScene = 0;

    // Finalize buffer
    model.buffers[bufferIndex].data = std::move(bufferData);

    // Write .glb (binary)
    tinygltf::TinyGLTF writer;
    if (!writer.WriteGltfSceneToFile(&model, fullPath, false, true, true, true))
    {
        std::cerr << "ModelExporter: Failed to write glTF file " << fullPath << std::endl;
        return {};
    }

    // Write .gltf (text) alongside for debugging
    std::string gltfPath = outputDir.pathname() + "/" + baseName + ".gltf";
    writer.WriteGltfSceneToFile(&model, gltfPath, false, false, true, false);

    return glbFileName;
}

void MachPhysModelExporter::writeSharedGltfFile(
    const SysPathName& outputDir,
    const std::string& baseName,
    const std::vector<std::pair<std::string, const RenMeshInstance*>>& meshes)
{
    if (meshes.empty())
        return;

    std::string glbFileName = baseName + ".glb";
    std::string fullPath = outputDir.pathname() + "/" + glbFileName;

    tinygltf::Model model;
    model.asset.version = "2.0";
    model.asset.generator = "Machines glTF Exporter";

    tinygltf::Buffer buffer;
    std::vector<unsigned char> bufferData;
    bufferData.reserve(64 * 1024);
    int bufferIndex = 0;
    model.buffers.push_back(buffer);

    // Texture deduplication map (textureName -> glTF texture index)
    std::map<std::string, int> textureMap;

    // Root node
    tinygltf::Node rootNode;
    rootNode.name = baseName;
    int rootNodeIdx = static_cast<int>(model.nodes.size());
    model.nodes.push_back(rootNode);

    for (const auto& [meshName, mi] : meshes)
    {
        if (!mi)
            continue;

        RenI::MeshData meshData = RenMesh::extractMeshData(*mi);
        if (meshData.primitives.empty())
            continue;

        std::vector<int> materialMap;
        for (const auto& md : meshData.materials)
            materialMap.push_back(addGltfMaterial(model, md, textureMap));

        int gltfMeshIdx = addGltfMesh(model, bufferIndex, bufferData, meshData, materialMap);

        tinygltf::Node node;
        node.name = meshName;
        node.mesh = gltfMeshIdx;

        int nodeIdx = static_cast<int>(model.nodes.size());
        model.nodes.push_back(std::move(node));
        model.nodes[rootNodeIdx].children.push_back(nodeIdx);
    }

    tinygltf::Scene scene;
    scene.name = baseName;
    scene.nodes.push_back(rootNodeIdx);
    model.scenes.push_back(scene);
    model.defaultScene = 0;

    model.buffers[bufferIndex].data = std::move(bufferData);

    tinygltf::TinyGLTF writer;
    if (!writer.WriteGltfSceneToFile(&model, fullPath, false, true, true, true))
    {
        std::cerr << "ModelExporter: Failed to write shared glTF file " << fullPath << std::endl;
    }

    // Write .gltf (text) alongside for debugging
    std::string gltfPath = outputDir.pathname() + "/" + baseName + ".gltf";
    writer.WriteGltfSceneToFile(&model, gltfPath, false, false, true, false);
}
