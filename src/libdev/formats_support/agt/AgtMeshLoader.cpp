#include "formats_support/agt/AgtMeshLoader.hpp"

#include "base/diag.hpp"
#include "system/pathname.hpp"

#include "gxin/gxcolor.hpp"
#include "gxin/gxerror.hpp"
#include "gxin/gxfile.hpp"
#include "gxin/gxhier.hpp"
#include "gxin/gxmat.hpp"
#include "gxin/gxmatrix3.hpp"
#include "gxin/gxmesh.hpp"
#include "gxin/gxpolygon3.hpp"
#include "gxin/gxpolyvert3.hpp"
#include "gxin/gxtexture.hpp"
#include "gxin/gxuvcoords.hpp"
#include "gxin/gxvect3.hpp"

#include <cmath>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// FileEntry -- per-file cache: holds GXMesh pointers, hierarchy, and name->MeshData map.
// ---------------------------------------------------------------------------

struct AgtMeshLoader::FileEntry
{
    // All meshes read from the file (we own them).
    std::vector<GXMesh*> meshes;
    // Hierarchy read from the file (we own it).
    GXHier* hierarchy{};
    // File path (used for hierarchy loading).
    SysPathName pathName;

    ~FileEntry()
    {
        for (auto* m : meshes)
            delete m;
        delete hierarchy;
    }
};

// ---------------------------------------------------------------------------
// Convert a single GXMesh into a RenI::MeshData.
//
// The AGT format stores:
//   - A flat array of 3D points and normals on the GXMesh.
//   - Polygons (always triangles in practice) that index into those arrays
//     via GXPolyVert3 (pointIndex, normalIndex, UV, optional per-vertex color).
//   - A per-polygon material (GXMat) carrying diffuse color, texture ID, and
//     transparency.
//
// The original buildFromGXMesh groups polygons by material, deduplicates
// vertices by (pointIndex, normalIndex), flips Y/Z, flips UV v, and flips
// triangle winding.  We reproduce that logic here but output RenI::MeshData
// instead of directly populating RenMesh internals.
// ---------------------------------------------------------------------------

static RenI::MeshData convertGXMesh(GXMesh* gxmesh)
{
    RenI::MeshData data;
    if (gxmesh->hasName())
        data.name = gxmesh->name().str();

    // Step 1: Group polygon indices by material.
    // The original code uses a map<GXMat, vector<int>> but matches shaded
    // (multi-color) polygons by texture ID only.  We reproduce that logic
    // by keeping a vector of (GXMat, polygonIndices) pairs and doing a
    // linear scan for shaded polygons (matching by texture ID).

    struct MaterialGroup
    {
        GXMat mat;
        std::vector<int> polygonIndices;
    };
    std::vector<MaterialGroup> groups;

    auto findGroupByTextureId = [&](const GXMat& mat) -> MaterialGroup*
    {
        for (auto& g : groups)
        {
            if (g.mat.textureId() == mat.textureId())
                return &g;
        }
        return nullptr;
    };

    auto findGroupByMat = [&](const GXMat& mat) -> MaterialGroup*
    {
        for (auto& g : groups)
        {
            if (g.mat == mat)
                return &g;
        }
        return nullptr;
    };

    for (int pi = 0; pi < gxmesh->numPolygons(); ++pi)
    {
        GXPolygon3 poly = gxmesh->polygon(pi);
        GXMat polyMat(poly.mat());

        if (!poly.hasOneColor())
        {
            // Shaded polygon: match by texture ID only.
            MaterialGroup* existing = findGroupByTextureId(polyMat);
            if (existing)
            {
                existing->polygonIndices.push_back(pi);
            }
            else
            {
                // Set the diffuse color from the first vertex so the material
                // entry has hasOneColor = true for the map key.
                if (poly.numVertices() > 0)
                    polyMat.diffuseColor(poly.vertex(0).color());

                MaterialGroup mg;
                mg.mat = polyMat;
                mg.polygonIndices.push_back(pi);
                groups.push_back(std::move(mg));
            }
        }
        else
        {
            // Flat-colored polygon: match by full material equality.
            MaterialGroup* existing = findGroupByMat(polyMat);
            if (existing)
            {
                existing->polygonIndices.push_back(pi);
            }
            else
            {
                MaterialGroup mg;
                mg.mat = polyMat;
                mg.polygonIndices.push_back(pi);
                groups.push_back(std::move(mg));
            }
        }
    }

    // Step 2: For each material group, build a RenI::MeshPrimitive.
    // Vertices are deduplicated within each primitive by (pointIndex, normalIndex).
    // Coordinates are flipped: engine Y <- AGT Z, engine Z <- AGT Y.
    // UV v is flipped: engine v = 1 - agt v.
    // Triangle winding is flipped: (v0, v2, v1) instead of (v0, v1, v2).

    int materialIdx = 0;
    for (const auto& group : groups)
    {
        // Build material.
        RenI::MeshMaterial mat;
        const GXMat& gxMat = group.mat;
        if (gxMat.hasOneColor())
        {
            const GXColor& c = gxMat.diffuseColor();
            mat.diffuseR = c.red() / 255.0f;
            mat.diffuseG = c.green() / 255.0f;
            mat.diffuseB = c.blue() / 255.0f;
            mat.diffuseA = (100.0f - gxMat.transparancy()) / 100.0f;
        }

        if (const_cast<GXMat&>(gxMat).hasValidTexture())
        {
            GXIdPos txId = gxMat.textureId();
            const GXTexture& tx = gxmesh->texture(txId);
            mat.textureName = tx.name().str();
        }

        data.materials.push_back(mat);

        // Build primitive.
        RenI::MeshPrimitive prim;
        prim.materialIndex = materialIdx;

        // Vertex deduplication map: (pointIndex, normalIndex) -> local vertex index.
        struct VertKey
        {
            int pointIdx{};
            int normalIdx{};
            bool operator==(const VertKey& o) const
            {
                return pointIdx == o.pointIdx && normalIdx == o.normalIdx;
            }
        };
        // Simple linear search (meshes are small).
        std::vector<std::pair<VertKey, uint32_t>> vertMap;

        auto getOrAddVertex = [&](const GXPolyVert3& gxv) -> uint32_t
        {
            VertKey key{gxv.pointIndex(), gxv.normalIndex()};
            for (const auto& [k, idx] : vertMap)
            {
                if (k == key)
                    return idx;
            }

            // New vertex -- convert coordinates with Y/Z flip.
            const GXPoint3& pt = gxmesh->point(gxv.pointIndex());
            const GXVect3& nm = gxmesh->normal(gxv.normalIndex());

            RenI::MeshVertex v;
            v.px = pt.x();
            v.py = pt.z(); // flip Y/Z
            v.pz = pt.y();
            v.nx = nm.x();
            v.ny = nm.z(); // flip Y/Z
            v.nz = nm.y();
            v.tu = gxv.uv().u();
            v.tv = 1.0f - gxv.uv().v(); // flip V

            // Handle zero normals (3D Studio sometimes creates them).
            float lenSq = v.nx * v.nx + v.ny * v.ny + v.nz * v.nz;
            if (lenSq < 1e-12f)
            {
                v.nx = 0.0f;
                v.ny = 0.0f;
                v.nz = 1e-6f;
            }

            auto idx = static_cast<uint32_t>(prim.vertices.size());
            prim.vertices.push_back(v);
            vertMap.push_back({key, idx});
            return idx;
        };

        for (int pi : group.polygonIndices)
        {
            GXPolygon3 poly = gxmesh->polygon(pi);
            // AGT meshes should always be triangulated.
            if (poly.numVertices() != 3)
                continue;

            uint32_t i0 = getOrAddVertex(poly.vertex(0));
            uint32_t i1 = getOrAddVertex(poly.vertex(1));
            uint32_t i2 = getOrAddVertex(poly.vertex(2));

            // Flip winding: (v0, v2, v1)
            prim.indices.push_back(i0);
            prim.indices.push_back(i2);
            prim.indices.push_back(i1);
        }

        data.primitives.push_back(std::move(prim));
        ++materialIdx;
    }

    return data;
}

// ---------------------------------------------------------------------------
// File loading -- parses the .agt file via GXFile, caches results.
// ---------------------------------------------------------------------------

AgtMeshLoader::FileEntry* AgtMeshLoader::loadFile(const SysPathName& pathName)
{
    auto it = files_.find(pathName.pathname());
    if (it != files_.end())
        return it->second.get();

    auto entry = std::make_unique<FileEntry>();
    entry->pathName = pathName;

    GXLabel gxlabel;
    GXIdPos gxid;
    GXFile gxfile;
    GXError gxerror;
    GXTextures gxtextures;
    entry->hierarchy = new GXHier;

    std::vector<GXMesh*> meshesBeingLoaded;
    meshesBeingLoaded.reserve(32);

    gxerror = gxfile.open(pathName.pathname());
    if (gxerror != NO_ERROR_AGT)
    {
        gxfile.reportAGTError(FILE_OPEN_AGT, pathName.pathname().c_str());
    }
    else
    {
        gxerror = gxfile.skipHeader();
        if (gxerror != NO_ERROR_AGT)
        {
            gxfile.reportAGTError(HEADER_NAME_AGT, nullptr);
        }
        else
        {
            do
            {
                gxerror = gxfile.readLabel(gxlabel, true);
                if (gxerror == NO_ERROR_AGT)
                {
                    switch (gxlabel.type())
                    {
                        case OBJECTS:
                        {
                            // Inline loadObjects logic.
                            GXIdPos objId;
                            GXLabel meshtype;
                            gxerror = gxfile.readGXIdPos(objId, true, true, nullptr);
                            if (gxerror == NO_ERROR_AGT)
                            {
                                gxerror = gxfile.findTag(SECTION_START, true);
                                if (gxerror == NO_ERROR_AGT)
                                {
                                    do
                                    {
                                        gxerror = gxfile.readLabel(meshtype, true);
                                        if (gxerror == NO_ERROR_AGT)
                                        {
                                            switch (meshtype.type())
                                            {
                                                case TRIMESH:
                                                case QUADMESH:
                                                {
                                                    auto* gxmesh = new GXMesh;
                                                    gxerror = gxfile.readMesh(*gxmesh, meshtype);
                                                    if (gxerror == NO_ERROR_AGT)
                                                        meshesBeingLoaded.push_back(gxmesh);
                                                    else
                                                        delete gxmesh;
                                                    break;
                                                }
                                                default:
                                                    gxerror = gxfile.skipSection();
                                                    break;
                                            }
                                        }
                                    } while (gxerror == NO_ERROR_AGT);

                                    if (gxerror == NO_ERROR_AGT || gxerror == TOKEN_AGT)
                                        gxerror = gxfile.findTag(SECTION_END, true);
                                }
                            }
                            break;
                        }
                        case ANIMATION:
                            gxerror = gxfile.skipSection();
                            break;
                        case TEXTURES:
                            gxerror = gxfile.readTextures(gxtextures);
                            break;
                        case HIERARCHY:
                            gxerror = gxfile.readHierarchy(*entry->hierarchy);
                            break;
                        default:
                            gxerror = gxfile.skipSection();
                            break;
                    }
                }
            } while (gxerror == NO_ERROR_AGT);
        }
    }

    gxfile.close();

    // Set texture names and mesh names from the hierarchy.
    for (auto* mesh : meshesBeingLoaded)
    {
        mesh->setTextures(gxtextures);
        mesh->setName(*entry->hierarchy);
    }

    entry->meshes = std::move(meshesBeingLoaded);

    auto* raw = entry.get();
    files_[pathName.pathname()] = std::move(entry);
    return raw;
}

// ---------------------------------------------------------------------------
// IMeshLoader interface
// ---------------------------------------------------------------------------

AgtMeshLoader::AgtMeshLoader() = default;
AgtMeshLoader::~AgtMeshLoader() = default;

std::vector<std::string> AgtMeshLoader::supportedExtensions() const
{
    return {"agt"};
}

RenI::MeshData AgtMeshLoader::loadMesh(const SysPathName& path, const std::string& meshName)
{
    FileEntry* entry = loadFile(path);
    if (!entry)
        return {};

    for (auto* mesh : entry->meshes)
    {
        if (mesh->hasName() && mesh->name().str() == meshName)
            return convertGXMesh(mesh);
    }

    return {};
}

// ---------------------------------------------------------------------------
// Hierarchy conversion -- ported from RenIGXMeshLoader::convertHierarchy/Node.
// Returns a RenI::HierarchyNode tree instead of calling RenHierarchyBuilder.
// ---------------------------------------------------------------------------

static void normalizeVec3(float& x, float& y, float& z)
{
    float len = std::sqrt(x * x + y * y + z * z);
    if (len > 1e-12f)
    {
        x /= len;
        y /= len;
        z /= len;
    }
}

// static
RenI::HierarchyNode AgtMeshLoader::convertNode(
    const GXHier& source,
    const std::string& filePath)
{
    RenI::HierarchyNode node;

    // Transform with Y/Z flip (matching the original).
    GXMatrix3 gxTransform = source.transform();
    GXVect3 gxTranslate = source.translate();

    node.xBasisX = gxTransform.xx();
    node.xBasisY = gxTransform.zx();
    node.xBasisZ = gxTransform.yx();
    node.yBasisX = gxTransform.xz();
    node.yBasisY = gxTransform.zz();
    node.yBasisZ = gxTransform.yz();
    node.zBasisX = gxTransform.xy();
    node.zBasisY = gxTransform.zy();
    node.zBasisZ = gxTransform.yy();

    normalizeVec3(node.xBasisX, node.xBasisY, node.xBasisZ);
    normalizeVec3(node.yBasisX, node.yBasisY, node.yBasisZ);
    normalizeVec3(node.zBasisX, node.zBasisY, node.zBasisZ);

    node.posX = gxTranslate.x();
    node.posY = gxTranslate.z();
    node.posZ = gxTranslate.y();

    std::string meshName = source.meshName().str();
    node.instanceName = meshName;
    node.meshName = meshName;
    node.filePath = filePath;
    node.scale = 1.0;

    for (int i = 0; i < source.nChildren(); ++i)
        node.children.push_back(convertNode(source.child(i), filePath));

    return node;
}

RenI::HierarchyData AgtMeshLoader::loadHierarchy(const SysPathName& path)
{
    RenI::HierarchyData result;

    FileEntry* entry = loadFile(path);
    if (!entry || !entry->hierarchy)
        return result;

    const GXHier& root = *entry->hierarchy;

    // The original convertHierarchy skips the root node's transform
    // and only converts its children.
    for (int i = 0; i < root.nChildren(); ++i)
        result.roots.push_back(convertNode(root.child(i), entry->pathName.pathname()));

    return result;
}

void AgtMeshLoader::deleteAll()
{
    files_.clear();
}
