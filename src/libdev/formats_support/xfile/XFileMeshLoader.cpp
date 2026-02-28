#include "formats_support/xfile/XFileMeshLoader.hpp"

#include "base/diag.hpp"
#include "system/pathname.hpp"

#include "xin/XFileHelper.hpp"
#include "xin/XFileParser.hpp"

#include "mathex/coordsys.hpp"
#include "mathex/point3d.hpp"
#include "mathex/quatern.hpp"
#include "mathex/transf3d.hpp"
#include "mathex/vec3.hpp"

#include "utility/string.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <string>

// ---------------------------------------------------------------------------
// FileEntry -- per-file cache: holds the XFile::Scene and name->MeshData map.
// ---------------------------------------------------------------------------

struct XFileMeshLoader::FileEntry
{
    XFile::Scene* scene{};
    SysPathName pathName;
};

// ---------------------------------------------------------------------------
// Convert XFile::Mesh -> RenI::MeshData
//
// The .x format has a single shared vertex buffer and per-material face
// groups.  We produce one RenI::MeshPrimitive per material, each with its
// own local vertex/index arrays (matching the MeshData model).
// ---------------------------------------------------------------------------

RenI::MeshData XFileMeshLoader::convertMesh(XFile::Scene* scene, XFile::Mesh* mesh)
{
    RenI::MeshData data;
    data.name = mesh->mName;

    size_t nPositions = mesh->mPositions.size();
    bool hasTexCoords = !mesh->mTexCoords[0].empty()
        && mesh->mTexCoords[0].size() == nPositions;

    // Build materials
    for (auto it = mesh->mMaterials.begin(); it != mesh->mMaterials.end(); ++it)
    {
        XFile::Material* material = nullptr;
        if (it->mIsReference)
            material = scene->mGlobalMaterials[it->sceneIndex];
        else
            material = &(*it);

        RenI::MeshMaterial mm;
        mm.diffuseR = material->mDiffuse.r;
        mm.diffuseG = material->mDiffuse.g;
        mm.diffuseB = material->mDiffuse.b;
        mm.diffuseA = material->mDiffuse.a;

        if (!material->mTextures.empty() && material->mTextures[0].mName.size() > 4)
            mm.textureName = material->mTextures[0].mName;

        // Sorting flags from emissiveCtrl.r
        float r = material->mEmissiveCtrl.r;
        if (r > 0.5f && r <= 1.5f)
            mm.sortMode = RenI::SortMode::InterMeshCoplanar;
        else if (r > 1.5f && r <= 2.5f)
            mm.sortMode = RenI::SortMode::IntraMeshAlpha;
        else if (r > 2.5f && r <= 3.5f)
            mm.sortMode = RenI::SortMode::AbsoluteAlphaNegative;
        else if (r > 3.5f && r <= 4.5f)
            mm.sortMode = RenI::SortMode::AbsoluteAlphaPositive;
        mm.sortPriority = static_cast<short>(material->mSortPriority);

        // Emissive factor from emissiveCtrl.g
        float g = material->mEmissiveCtrl.g;
        mm.emissiveFactor = g;
        mm.emissiveR = g * mm.diffuseR;
        mm.emissiveG = g * mm.diffuseG;
        mm.emissiveB = g * mm.diffuseB;

        // STF axis and backface culling from mFlags.r
        float flags = material->mFlags.r;
        if (flags >= 1.5f && flags < 2.5f)
            mm.spinAxis = RenI::SpinAxis::X;
        else if (flags >= 2.5f && flags < 3.5f)
            mm.spinAxis = RenI::SpinAxis::Y;
        else if (flags >= 3.5f && flags < 4.5f)
            mm.spinAxis = RenI::SpinAxis::Z;
        mm.backfaceCull = (flags < 6.0f);

        data.materials.push_back(std::move(mm));
    }

    size_t nMaterials = data.materials.size();
    if (nMaterials == 0)
        return data;

    // Group faces by material.
    // Each face has 3 indices into the shared position/normal/UV arrays.
    std::vector<std::vector<size_t>> facesByMaterial(nMaterials);
    for (size_t i = 0; i < mesh->mPosFaces.size(); ++i)
    {
        size_t matIdx = mesh->mFaceMaterials[i];
        if (matIdx < nMaterials)
            facesByMaterial[matIdx].push_back(i);
    }

    // Build one primitive per material group.
    for (size_t mi = 0; mi < nMaterials; ++mi)
    {
        const auto& faces = facesByMaterial[mi];
        if (faces.empty())
            continue;

        RenI::MeshPrimitive prim;
        prim.materialIndex = static_cast<int>(mi);

        // Map from global vertex index -> local primitive vertex index.
        std::map<uint32_t, uint32_t> vertexRemap;

        for (size_t faceIdx : faces)
        {
            const auto& face = mesh->mPosFaces[faceIdx];
            // .x files should be triangulated at this point.
            for (size_t vi = 0; vi < 3 && vi < face.mIndices.size(); ++vi)
            {
                uint32_t globalIdx = static_cast<uint32_t>(face.mIndices[vi]);

                auto [it, inserted] = vertexRemap.try_emplace(
                    globalIdx,
                    static_cast<uint32_t>(prim.vertices.size()));

                if (inserted)
                {
                    RenI::MeshVertex v;
                    if (globalIdx < nPositions)
                    {
                        v.px = mesh->mPositions[globalIdx].x;
                        v.py = mesh->mPositions[globalIdx].y;
                        v.pz = mesh->mPositions[globalIdx].z;
                    }
                    if (globalIdx < mesh->mNormals.size())
                    {
                        v.nx = mesh->mNormals[globalIdx].x;
                        v.ny = mesh->mNormals[globalIdx].y;
                        v.nz = mesh->mNormals[globalIdx].z;
                    }
                    else if (!mesh->mNormals.empty())
                    {
                        v.nx = mesh->mNormals[0].x;
                        v.ny = mesh->mNormals[0].y;
                        v.nz = mesh->mNormals[0].z;
                    }
                    if (hasTexCoords)
                    {
                        v.tu = mesh->mTexCoords[0][globalIdx].x;
                        v.tv = mesh->mTexCoords[0][globalIdx].y;
                    }
                    prim.vertices.push_back(v);
                }

                prim.indices.push_back(it->second);
            }
        }

        data.primitives.push_back(std::move(prim));
    }

    return data;
}

// ---------------------------------------------------------------------------
// Populate HierarchyNode transform from an XFile::Node's 4x4 matrix.
// ---------------------------------------------------------------------------

static void nodeTransform(XFile::Node* frame, RenI::HierarchyNode& node)
{
    float(*m)[4][4] = &frame->mTrafoMatrix.data;

    MexVec3 xBasis((*m)[0][0], (*m)[0][1], (*m)[0][2]);
    MexVec3 yBasis((*m)[1][0], (*m)[1][1], (*m)[1][2]);
    MexVec3 zBasis((*m)[2][0], (*m)[2][1], (*m)[2][2]);

    const int yIdx = (MexCoordSystem::instance() == MexCoordSystem::RIGHT_HANDED) ? 2 : 1;
    const int zIdx = (MexCoordSystem::instance() == MexCoordSystem::RIGHT_HANDED) ? 1 : 2;
    MexPoint3d position((*m)[3][0], (*m)[3][yIdx], (*m)[3][zIdx]);

    MATHEX_SCALAR frameScale = xBasis.modulus();
    node.scale = frameScale;
    if (fabs(frameScale - 1.0) > 0.00001)
    {
        xBasis /= frameScale;
        yBasis /= frameScale;
        zBasis /= frameScale;
    }

    MexTransform3d t(xBasis, yBasis, zBasis, position);

    if (MexCoordSystem::instance() == MexCoordSystem::RIGHT_HANDED)
    {
        MexQuaternion q = t.rotationAsQuaternion();
        const MexQuaternion newQuat(q.vector().x(), q.vector().z(), q.vector().y(), -q.scalar());
        t.rotation(newQuat);
    }

    // Extract basis vectors and position from the final transform.
    MexVec3 bx, by, bz;
    t.xBasis(&bx);
    t.yBasis(&by);
    t.zBasis(&bz);
    MexPoint3d pos = t.position();

    node.xBasisX = bx.x(); node.xBasisY = bx.y(); node.xBasisZ = bx.z();
    node.yBasisX = by.x(); node.yBasisY = by.y(); node.yBasisZ = by.z();
    node.zBasisX = bz.x(); node.zBasisY = bz.y(); node.zBasisZ = bz.z();
    node.posX = pos.x(); node.posY = pos.y(); node.posZ = pos.z();
}

// ---------------------------------------------------------------------------
// XFileMeshLoader implementation
// ---------------------------------------------------------------------------

XFileMeshLoader::XFileMeshLoader() = default;

XFileMeshLoader::~XFileMeshLoader()
{
    deleteAll();
}

std::vector<std::string> XFileMeshLoader::supportedExtensions() const
{
    return {"x"};
}

void XFileMeshLoader::deleteAll()
{
    for (auto& [key, entry] : files_)
    {
        if (entry && entry->scene)
        {
            delete entry->scene;
            entry->scene = nullptr;
        }
    }
    files_.clear();
}

RenI::MeshData XFileMeshLoader::loadMesh(
    const SysPathName& pathName,
    const std::string& meshName)
{
    auto fileIt = files_.find(pathName.pathname());
    FileEntry* entry = nullptr;

    if (fileIt != files_.end())
        entry = fileIt->second.get();
    else
        entry = loadFile(pathName);

    if (!entry || !entry->scene)
        return {};

    std::string lcName(meshName);
    Utils::toLowerInPlace(&lcName);

    // Search frame-attached meshes.
    XFile::Mesh* found = nullptr;
    std::function<void(XFile::Node*)> search = [&](XFile::Node* frame)
    {
        if (found)
            return;
        if (!frame->mMeshes.empty())
        {
            std::string combinedName = frame->mName + 'X' + frame->mMeshes[0]->mName;
            std::string lcCombined(combinedName);
            Utils::toLowerInPlace(&lcCombined);
            if (lcCombined == lcName)
            {
                found = frame->mMeshes[0];
                return;
            }
            // Also match on raw mesh name.
            std::string rawName = frame->mMeshes[0]->mName;
            Utils::toLowerInPlace(&rawName);
            if (rawName == lcName)
            {
                found = frame->mMeshes[0];
                return;
            }
        }
        for (auto* child : frame->mChildren)
            search(child);
    };

    if (entry->scene->mRootNode)
        search(entry->scene->mRootNode);

    // Search global meshes.
    if (!found)
    {
        for (auto* globalMesh : entry->scene->mGlobalMeshes)
        {
            std::string lcGlobal(globalMesh->mName);
            Utils::toLowerInPlace(&lcGlobal);
            if (lcGlobal == lcName)
            {
                found = globalMesh;
                break;
            }
        }
    }

    if (found)
        return convertMesh(entry->scene, found);

    return {};
}

static RenI::HierarchyNode buildHierarchyNode(
    XFile::Node* frame,
    const std::string& filePath)
{
    RenI::HierarchyNode node;
    nodeTransform(frame, node);
    node.instanceName = frame->mName;
    node.filePath = filePath;

    if (!frame->mMeshes.empty())
    {
        std::string meshName = frame->mMeshes[0]->mName;
        node.meshName = frame->mName + 'X' + meshName;
    }

    for (auto* child : frame->mChildren)
        node.children.push_back(buildHierarchyNode(child, filePath));

    return node;
}

RenI::HierarchyData XFileMeshLoader::loadHierarchy(const SysPathName& pathName)
{
    RenI::HierarchyData result;

    auto fileIt = files_.find(pathName.pathname());
    if (fileIt == files_.end())
        loadFile(pathName);

    fileIt = files_.find(pathName.pathname());
    if (fileIt == files_.end())
        return result;

    FileEntry& entry = *fileIt->second;
    if (entry.scene && entry.scene->mRootNode)
        result.roots.push_back(buildHierarchyNode(entry.scene->mRootNode, pathName.pathname()));

    for (auto* globalMesh : entry.scene->mGlobalMeshes)
    {
        RenI::HierarchyNode node;
        node.instanceName = globalMesh->mName;
        node.meshName = globalMesh->mName;
        node.filePath = pathName.pathname();
        node.scale = 1.0;
        result.roots.push_back(std::move(node));
    }

    return result;
}

XFileMeshLoader::FileEntry* XFileMeshLoader::loadFile(const SysPathName& pathName)
{
    auto entry = std::make_unique<FileEntry>();
    entry->pathName = pathName;

    try
    {
        XFileParser xParser;
        xParser.LoadFromFile(pathName.pathname());
        entry->scene = xParser.GetImportedData();
    }
    catch (const XFile::DeadlyImportError& err)
    {
        std::cerr << "[XFILE] Failed to parse " << pathName << ": " << err.what() << std::endl;
        return nullptr;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[XFILE] Exception loading " << pathName << ": " << ex.what() << std::endl;
        return nullptr;
    }
    catch (...)
    {
        std::cerr << "[XFILE] Unknown exception loading " << pathName << std::endl;
        return nullptr;
    }

    XFile::Scene* scene = entry->scene;
    if (!scene)
        return nullptr;

    // No pre-conversion -- meshes are converted on-the-fly in loadMesh().

    FileEntry* raw = entry.get();
    files_[pathName.pathname()] = std::move(entry);
    return raw;
}

