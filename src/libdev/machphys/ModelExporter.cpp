#include "machphys/ModelExporter.hpp"

#include "mathex/double.hpp"
#include "mathex/transf3d.hpp"
#include "mathex/point3d.hpp"
#include "mathex/vec3.hpp"

#include "formats_support/MeshData.hpp"

#include "render/mesh.hpp"
#include "render/meshinst.hpp"
#include "render/material.hpp"
#include "render/colour.hpp"
#include "render/texture.hpp"

#include "ctl/list.hpp"
#include "ctl/vector.hpp"

#include "world4d/Entity/Entity.hpp"
#include "world4d/Entity/Composite.hpp"
#include "world4d/Entity/CompositePlan.hpp"
#include "world4d/Entity/EntityPlan.hpp"
#include "world4d/Scene/Internal/LightData.hpp"
#include "world4d/Materials/Internal/MultiTexture.hpp"
#include "world4d/Materials/Internal/CycleTextureData.hpp"
#include "world4d/Materials/Internal/UVData.hpp"
#include "world4d/Materials/Internal/MultiColour.hpp"
#include "world4d/Plans/ColourPulseData.hpp"
#include "world4d/Materials/Internal/AnimationData.hpp"
#include "world4d/Entity/Link.hpp"
#include "world4d/Entity/CompositePlanEntry.hpp"
#include "world4d/Entity/Root.hpp"

#include "mathex/quatern.hpp"
#include "mathex/vec2.hpp"

#include "phys/Plans/MotionPlan.hpp"

#include "machphys/Persistence/Persistence.hpp"
#include "machphys/Terrain/PlanetSurface.hpp"
#include "machphys/Terrain/TerrainTile.hpp"

#include "system/pathname.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// Helper: write a 4x4 transform matrix in .x file format.
// The game uses LEFT_HANDED coordinate system (default, never changed),
// so the loader does NOT apply any Y/Z swaps.  Write directly.
static void writeTransformMatrix(std::ostream& os, const MexTransform3d& t, const std::string& indent)
{
    MexPoint3d pos = t.position();

    MexVec3 xBasis, yBasis, zBasis;
    t.xBasis(&xBasis);
    t.yBasis(&yBasis);
    t.zBasis(&zBasis);

    os << indent << "FrameTransformMatrix {\n";
    os << indent << "  " << std::fixed << std::setprecision(6)
       << xBasis.x() << ", " << xBasis.y() << ", " << xBasis.z() << ", 0.000000,\n";
    os << indent << "  "
       << yBasis.x() << ", " << yBasis.y() << ", " << yBasis.z() << ", 0.000000,\n";
    os << indent << "  "
       << zBasis.x() << ", " << zBasis.y() << ", " << zBasis.z() << ", 0.000000,\n";
    os << indent << "  "
       << pos.x() << ", " << pos.y() << ", " << pos.z() << ", 1.000000;;\n";
    os << indent << "}\n";
}

// Convert SortMode enum to the .x emissiveCtrl.r float value.
static float sortModeToXFloat(RenI::SortMode mode)
{
    switch (mode)
    {
    case RenI::SortMode::InterMeshCoplanar:
        return 1.0f;
    case RenI::SortMode::IntraMeshAlpha:
        return 2.0f;
    case RenI::SortMode::AbsoluteAlphaNegative:
        return 3.0f;
    case RenI::SortMode::AbsoluteAlphaPositive:
        return 4.0f;
    case RenI::SortMode::None:
        return 0.0f;
    }
    return 0.0f;
}

// Convert SpinAxis enum to the .x specular red float value.
static float spinAxisToXFloat(RenI::SpinAxis axis)
{
    switch (axis)
    {
    case RenI::SpinAxis::X:
        return 2.0f;
    case RenI::SpinAxis::Y:
        return 3.0f;
    case RenI::SpinAxis::Z:
        return 4.0f;
    case RenI::SpinAxis::None:
        return 0.0f;
    }
    return 0.0f;
}

// Write a .x Material block from a MeshMaterial.
// Converts clean enum-based fields back to the legacy .x float encoding.
static void writeXMaterial(
    std::ostream& os,
    const RenI::MeshMaterial& md,
    const std::string& indent)
{
    float xSortMode = sortModeToXFloat(md.sortMode);
    float xSpecularRed = spinAxisToXFloat(md.spinAxis);

    os << indent << "Material {\n";
    os << indent << "  " << std::fixed << std::setprecision(6)
       << md.diffuseR << "; " << md.diffuseG << "; " << md.diffuseB << "; " << md.diffuseA << ";;\n";
    os << indent << "  " << static_cast<float>(md.sortPriority) << ";\n";
    os << indent << "  " << xSpecularRed << "; 0.000000; 0.000000;;\n";
    os << indent << "  " << xSortMode << "; " << md.emissiveFactor << "; 0.000000;;\n";

    if (!md.textureName.empty())
    {
        os << indent << "  TextureFilename {\n";
        os << indent << "    \"" << md.textureName << "\";\n";
        os << indent << "  }\n";
    }

    os << indent << "}\n";
}

void MachPhysModelExporter::writeMesh(
    std::ostream& os,
    const RenMeshInstance& meshInst,
    const std::string& indent)
{
    RenI::MeshData data = RenMesh::extractMeshData(meshInst);
    if (data.primitives.empty())
        return;

    // The .x format uses a single global vertex buffer shared across all face
    // groups.  Flatten per-primitive local vertices into one array, remapping
    // each primitive's indices by the running vertex offset.
    struct FlatPrimitive
    {
        size_t vertexOffset{};
        size_t nTris{};
        int materialIndex{-1};
    };

    std::vector<RenI::MeshVertex> allVerts;
    std::vector<FlatPrimitive> flatPrims;

    for (const auto& prim : data.primitives)
    {
        FlatPrimitive fp;
        fp.vertexOffset = allVerts.size();
        fp.nTris = prim.indices.size() / 3;
        fp.materialIndex = prim.materialIndex;
        flatPrims.push_back(fp);

        allVerts.insert(allVerts.end(), prim.vertices.begin(), prim.vertices.end());
    }

    size_t totalVerts = allVerts.size();
    size_t totalTris = 0;
    for (const auto& fp : flatPrims)
        totalTris += fp.nTris;

    if (totalTris == 0)
        return;

    os << indent << "Mesh " << data.name << " {\n";

    // Vertices
    os << indent << "  " << totalVerts << ";\n";
    for (size_t i = 0; i < totalVerts; ++i)
    {
        const auto& v = allVerts[i];
        os << indent << "  " << std::fixed << std::setprecision(6)
           << v.px << "; " << v.py << "; " << v.pz << ";";
        os << (i + 1 < totalVerts ? "," : "") << "\n";
    }

    // Faces
    os << indent << "  " << totalTris << ";\n";
    size_t faceIdx = 0;
    for (size_t pi = 0; pi < data.primitives.size(); ++pi)
    {
        const auto& prim = data.primitives[pi];
        size_t off = flatPrims[pi].vertexOffset;

        for (size_t t = 0; t < prim.indices.size(); t += 3)
        {
            os << indent << "  3; "
               << (off + prim.indices[t]) << ", "
               << (off + prim.indices[t + 1]) << ", "
               << (off + prim.indices[t + 2]) << ";";
            ++faceIdx;
            os << (faceIdx < totalTris ? "," : "") << "\n";
        }
    }

    // MeshMaterialList
    os << indent << "  MeshMaterialList {\n";
    os << indent << "    " << data.materials.size() << ";\n";
    os << indent << "    " << totalTris << ";\n";

    faceIdx = 0;
    for (const auto& fp : flatPrims)
    {
        for (size_t t = 0; t < fp.nTris; ++t)
        {
            os << indent << "    " << fp.materialIndex;
            ++faceIdx;
            os << (faceIdx < totalTris ? "," : ";;") << "\n";
        }
    }

    for (const auto& mat : data.materials)
        writeXMaterial(os, mat, indent + "    ");

    os << indent << "  }\n"; // end MeshMaterialList

    // MeshNormals
    os << indent << "  MeshNormals {\n";
    os << indent << "    " << totalVerts << ";\n";
    for (size_t i = 0; i < totalVerts; ++i)
    {
        const auto& v = allVerts[i];
        os << indent << "    " << std::fixed << std::setprecision(6)
           << v.nx << "; " << v.ny << "; " << v.nz << ";";
        os << (i + 1 < totalVerts ? "," : "") << "\n";
    }

    // Normal face indices (same as position faces)
    os << indent << "    " << totalTris << ";\n";
    faceIdx = 0;
    for (size_t pi = 0; pi < data.primitives.size(); ++pi)
    {
        const auto& prim = data.primitives[pi];
        size_t off = flatPrims[pi].vertexOffset;

        for (size_t t = 0; t < prim.indices.size(); t += 3)
        {
            os << indent << "    3; "
               << (off + prim.indices[t]) << ", "
               << (off + prim.indices[t + 1]) << ", "
               << (off + prim.indices[t + 2]) << ";";
            ++faceIdx;
            os << (faceIdx < totalTris ? "," : "") << "\n";
        }
    }
    os << indent << "  }\n"; // end MeshNormals

    // MeshTextureCoords
    os << indent << "  MeshTextureCoords {\n";
    os << indent << "    " << totalVerts << ";\n";
    for (size_t i = 0; i < totalVerts; ++i)
    {
        const auto& v = allVerts[i];
        os << indent << "    " << std::fixed << std::setprecision(6)
           << v.tu << "; " << v.tv << ";";
        os << (i + 1 < totalVerts ? "," : "") << "\n";
    }
    os << indent << "  }\n"; // end MeshTextureCoords

    os << indent << "}\n"; // end Mesh
}

W4dLOD MachPhysModelExporter::maxLODCount(const W4dComposite& composite)
{
    W4dLOD maxLod = composite.nLODs();
    for (const W4dLink* link : composite.links())
    {
        if (link)
        {
            W4dLOD n = link->nLODs();
            if (n > maxLod)
                maxLod = n;
        }
    }
    return maxLod;
}

// Extract the original .x leaf filename (without extension) from the first
// mesh's pathName_ at the given LOD.  The game's RenMesh::pathName_ stores
// the original filename from models.bin (e.g. "models/administ/boss/level1/agf1a20.x").
// Returns the leaf without extension (e.g. "agf1a20"), or empty if no mesh found.
static std::string originalMeshBaseName(const W4dComposite& composite, W4dLOD lod)
{
    auto extractBase = [](const W4dEntity& ent, W4dLOD l) -> std::string
    {
        if (!ent.hasMesh(l))
            return {};
        const RenMeshInstance& mi = ent.mesh(l);
        Ren::ConstMeshPtr mp = mi.mesh();
        if (!mp.isDefined() || !mp->pathName().set())
            return {};
        std::string pn = mp->pathName().pathname();
        // Extract leaf filename
        auto slash = pn.rfind('/');
        std::string leaf = (slash != std::string::npos) ? pn.substr(slash + 1) : pn;
        // Strip extension
        auto dot = leaf.rfind('.');
        if (dot != std::string::npos)
            leaf = leaf.substr(0, dot);
        return leaf;
    };

    // Try composite root first
    std::string base = extractBase(composite, lod);
    if (!base.empty())
        return base;

    // Try links
    for (const W4dLink* link : composite.links())
    {
        if (!link)
            continue;
        base = extractBase(*link, lod);
        if (!base.empty())
            return base;
    }
    return {};
}

// Check if a LOD level has any meshes across composite + links
static bool lodHasAnyMesh(const W4dComposite& composite, W4dLOD lod)
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

std::string MachPhysModelExporter::writeXFile(
    const W4dComposite& composite,
    const SysPathName& outputDir,
    const std::string& baseName,
    W4dLOD lod)
{
    if (!lodHasAnyMesh(composite, lod))
        return {};

    std::string xFileName = baseName + ".x";

    std::string fullPath = outputDir.pathname() + "/" + xFileName;

    std::ofstream ofs(fullPath);
    if (!ofs.is_open())
    {
        std::cerr << "ModelExporter: Failed to create " << fullPath << std::endl;
        return {};
    }

    ofs << "xof 0303txt 0032\n\n";

    const std::string& rootName = composite.name();
    std::string safeRootName = rootName.empty() ? baseName : rootName;

    ofs << "Frame " << safeRootName << " {\n";

    writeTransformMatrix(ofs, composite.localTransform(), "  ");

    if (composite.hasMesh(lod))
    {
        const RenMeshInstance& meshInst = composite.mesh(lod);
        writeMesh(ofs, meshInst, "  ");
    }

    // Build parent->children map from all links
    const auto& links = composite.links();
    std::map<const W4dEntity*, std::vector<const W4dLink*>> linkTree;
    for (auto* link : links)
    {
        if (!link)
            continue;
        const W4dEntity* parent = link->hasParent() ? link->pParent() : nullptr;
        linkTree[parent].push_back(link);
    }

    std::function<void(const W4dEntity*, const std::string&)> writeLinksRecursive;
    writeLinksRecursive = [&](const W4dEntity* parent, const std::string& indent)
    {
        auto it = linkTree.find(parent);
        if (it == linkTree.end())
            return;

        for (const W4dLink* link : it->second)
        {
            const std::string& linkName = link->name();
            std::string safeLinkName = linkName.empty() ? "link" : linkName;

            {
                MexPoint3d ep = link->localTransform().position();
                MexQuaternion eq = link->localTransform().rotationAsQuaternion();
                std::cout << "[EXPORT]   link '" << safeLinkName
                          << "' enginePos=(" << ep.x() << ", " << ep.y() << ", " << ep.z()
                          << ") engineQuat=(" << eq.vector().x() << ", " << eq.vector().y()
                          << ", " << eq.vector().z() << ", " << eq.scalar() << ")" << std::endl;
            }

            ofs << indent << "Frame " << safeLinkName << " {\n";

            writeTransformMatrix(ofs, link->localTransform(), indent + "  ");

            if (link->hasMesh(lod))
            {
                const RenMeshInstance& meshInst = link->mesh(lod);
                writeMesh(ofs, meshInst, indent + "  ");
            }

            writeLinksRecursive(link, indent + "  ");

            ofs << indent << "}\n";
        }
    };

    writeLinksRecursive(&composite, "  ");

    ofs << "}\n";

    ofs.close();
    return xFileName;
}

// Write a single GENERIC_LIGHT block for the given light data.
// ind = indent for the GENERIC_LIGHT line, ind2 = indent for properties inside it.
static void writeLightBlock(std::ostream& ofs, const W4dLightData& ld,
    const char* ind, const char* ind2)
{
    ofs << ind << "GENERIC_LIGHT\n";

    // TYPE
    switch (ld.type())
    {
        case W4dLightData::POINT:
            ofs << ind2 << "TYPE POINT\n";
            break;
        case W4dLightData::UNIFORM:
            ofs << ind2 << "TYPE UNIFORM\n";
            break;
        case W4dLightData::DIRECTIONAL:
            ofs << ind2 << "TYPE DIRECTIONAL\n";
            break;
    }

    // SCOPE
    switch (ld.scope())
    {
        case W4dLightData::LOCAL_LIGHT:
            ofs << ind2 << "SCOPE LOCAL_LIGHT\n";
            break;
        case W4dLightData::GLOBAL_LIGHT:
            ofs << ind2 << "SCOPE GLOBAL_LIGHT\n";
            break;
        case W4dLightData::DOMAIN_LIGHT:
            ofs << ind2 << "SCOPE DOMAIN_LIGHT\n";
            break;
        case W4dLightData::DOMAIN_COMPOSITE:
            ofs << ind2 << "SCOPE DOMAIN_COMPOSITE\n";
            break;
        case W4dLightData::DYNAMIC_LIGHT:
            ofs << ind2 << "SCOPE DYNAMIC_LIGHT\n";
            break;
    }

    // DUMMY_MESH or POSITION
    if (!ld.name().empty())
    {
        ofs << ind2 << "DUMMY_MESH " << ld.name() << "\n";
    }
    else
    {
        const MexPoint3d& pos = ld.position();
        ofs << ind2 << "POSITION " << std::fixed << std::setprecision(6)
            << pos.x() << " " << pos.y() << " " << pos.z() << "\n";
    }

    // COLOUR
    const RenColour& col = ld.colour();
    ofs << ind2 << "COLOUR " << std::fixed << std::setprecision(6)
        << col.r() << " " << col.g() << " " << col.b() << "\n";

    // DIRECTION
    const MexVec3& dir = ld.direction();
    ofs << ind2 << "DIRECTION " << std::fixed << std::setprecision(6)
        << dir.x() << " " << dir.y() << " " << dir.z() << "\n";

    // RANGE + ATTENUATIONS (for POINT and UNIFORM)
    if (ld.type() == W4dLightData::POINT || ld.type() == W4dLightData::UNIFORM)
    {
        ofs << ind2 << "RANGE " << std::fixed << std::setprecision(6) << ld.maxRange() << "\n";

        MATHEX_SCALAR a1{}, a2{}, a3{};
        ld.attenuations(&a1, &a2, &a3);
        ofs << ind2 << "ATTENUATIONS " << std::fixed << std::setprecision(6)
            << a1 << " " << a2 << " " << a3 << "\n";
    }

    // TIMES + INTENSITIES
    const auto& times = ld.times();
    const auto& intensities = ld.intensities();
    if (!times.empty())
    {
        ofs << ind2 << "TIMES";
        for (size_t t = 0; t < times.size(); ++t)
            ofs << " " << std::fixed << std::setprecision(6) << times[t];
        ofs << "\n";

        ofs << ind2 << "INTENSITIES";
        for (size_t t = 0; t < intensities.size(); ++t)
            ofs << " " << std::fixed << std::setprecision(6) << intensities[t];
        ofs << "\n";
    }

    ofs << ind << "END\n";
}

// Write ANIMATION_DATA block for non-light animation data on an entity.
// Returns true if any data was written.
static bool writeAnimationDataBlock(std::ostream& ofs, const W4dEntity& entity)
{
    if (entity.nAnimationData() == 0 && entity.nLightData() == 0)
        return false;

    ofs << "ANIMATION_DATA\n";

    for (size_t i = 0; i < entity.nAnimationData(); ++i)
    {
        const W4dAnimationData& ad = entity.animationData(i);

        if (const auto* cycleMulti = dynamic_cast<const W4dCycleMultiTextureData*>(&ad))
        {
            const auto& vec = cycleMulti->cycleTextureDataVec();
            ofs << "\tCYCLE_TEXTURE " << vec.size() << "\n";
            for (const auto* ctd : vec)
            {
                ofs << "\t\t" << ctd->textureName() << " "
                    << ctd->startTexture() << " "
                    << ctd->endTexture() << " FRAMES "
                    << ctd->nRepetations() << " MAX_LOD "
                    << ad.maxLod() << "\n";
            }
            ofs << "\tEND\n";
        }
        else if (const auto* uvTrans = dynamic_cast<const W4dUVTranslateData*>(&ad))
        {
            ofs << "\tUV_TRANSLATE " << ad.name() << " "
                << std::fixed << std::setprecision(6)
                << uvTrans->speedVec().x() << " "
                << uvTrans->speedVec().y() << " MAX_LOD "
                << ad.maxLod() << "\n";
        }
        else if (const auto* colMulti = dynamic_cast<const W4dMultiColourPulseData*>(&ad))
        {
            const auto& vec = colMulti->colourPulseDataVec();
            ofs << "\tCOLOUR_PULSE " << vec.size() << "\n";
            for (const auto* cpd : vec)
            {
                const RenColour& from = cpd->fromColour();
                const RenColour& to = cpd->toColour();
                ofs << "\t\t" << cpd->keyTexture().name()
                    << " FROM "
                    << std::fixed << std::setprecision(1)
                    << (from.r() * 255.0) << " " << (from.g() * 255.0) << " " << (from.b() * 255.0)
                    << " TO "
                    << (to.r() * 255.0) << " " << (to.g() * 255.0) << " " << (to.b() * 255.0)
                    << " EVERY "
                    << std::setprecision(6) << cpd->duration()
                    << " MAX_LOD " << ad.maxLod() << "\n";
            }
            ofs << "\tEND\n";
        }
    }

    // GENERIC_LIGHT entries also go inside ANIMATION_DATA
    for (size_t i = 0; i < entity.nLightData(); ++i)
    {
        writeLightBlock(ofs, entity.lightData(i), "\t", "\t\t");
    }

    ofs << "END\n";
    return true;
}

// Write a shadow's mesh to a standalone .x file. Returns the mesh name, or empty if no mesh found.
static std::string writeShadowXFile(
    std::ostream& os,
    const W4dEntity& shadowEntity)
{
    // For SHADOW_FIXED, the mesh is on the entity itself.
    // For SHADOW_PROJ, the mesh is on its first child (a W4dGeneric).
    const W4dEntity* meshOwner = nullptr;
    if (shadowEntity.hasMesh())
    {
        meshOwner = &shadowEntity;
    }
    else
    {
        for (const W4dEntity* child : shadowEntity.children())
        {
            if (child && child->hasMesh())
            {
                meshOwner = child;
                break;
            }
        }
    }

    if (!meshOwner)
        return {};

    const RenMeshInstance& meshInst = meshOwner->mesh(0);
    Ren::ConstMeshPtr pMesh = meshInst.mesh();
    if (!pMesh.isDefined())
        return {};

    std::string meshName = pMesh->meshName();
    if (meshName.empty())
        meshName = "shadow";

    os << "xof 0303txt 0032\n\n";
    os << "Frame shadow_root {\n";
    os << "  FrameTransformMatrix {\n";
    os << "    1.000000, 0.000000, 0.000000, 0.000000,\n";
    os << "    0.000000, 1.000000, 0.000000, 0.000000,\n";
    os << "    0.000000, 0.000000, 1.000000, 0.000000,\n";
    os << "    0.000000, 0.000000, 0.000000, 1.000000;;\n";
    os << "  }\n";
    MachPhysModelExporter::writeMesh(os, meshInst, "  ");
    os << "}\n";

    return meshName;
}

void MachPhysModelExporter::writeCdfFile(
    const W4dComposite& composite,
    const SysPathName& outputDir,
    const std::string& baseName,
    const std::vector<std::string>& xFileNames,
    const std::vector<double>& distances,
    const std::vector<AnimInfo>& animations)
{
    std::string fullPath = outputDir.pathname() + "/" + baseName + ".cdf";

    std::ofstream ofs(fullPath);
    if (!ofs.is_open())
    {
        std::cerr << "ModelExporter: Failed to create " << fullPath << std::endl;
        return;
    }

    // Write MODEL + DISTANCE lines for each LOD
    for (size_t i = 0; i < xFileNames.size(); ++i)
    {
        if (xFileNames[i].empty())
            continue;

        ofs << "MODEL " << xFileNames[i] << "\n";

        if (i < distances.size() && !std::isinf(distances[i]))
        {
            ofs << "\tDISTANCE " << std::fixed << std::setprecision(1) << distances[i] << "\n";
        }
    }

    // Write MESH_RANGES for links that have per-link distance overrides
    // A link needs MESH_RANGES if it has a meshDistance that differs from
    // the default LOD distances written in the MODEL sections above.
    for (const W4dLink* link : composite.links())
    {
        if (!link || link->name().empty())
            continue;

        // Collect distances for each LOD of this link
        W4dLOD nLods = link->nLODs();
        if (nLods <= 1)
            continue;

        std::vector<double> linkDists;
        bool hasDifferentDist = false;
        for (W4dLOD j = 0; j < nLods; ++j)
        {
            if (!link->hasMesh(j))
                continue;
            double d = link->meshDistance(j);
            linkDists.push_back(d);
            // Check if this link has a non-default distance
            if (static_cast<size_t>(j) < distances.size() && d != distances[j])
                hasDifferentDist = true;
        }

        if (hasDifferentDist)
        {
            ofs << "MESH_RANGES " << link->name();
            for (size_t j = 0; j < linkDists.size(); ++j)
            {
                if (!std::isinf(linkDists[j]))
                    ofs << " " << std::fixed << std::setprecision(1) << linkDists[j];
            }
            ofs << "\n";
        }
    }

    // Write SHADOW_FIXED / SHADOW_PROJ lines.
    // Shadows are child entities of the composite with name "SHADOW_FIXED" or "SHADOW_PROJ".
    int shadowIdx = 0;
    for (const W4dEntity* child : composite.children())
    {
        if (!child)
            continue;
        const std::string& childName = child->name();
        if (childName != "SHADOW_FIXED" && childName != "SHADOW_PROJ")
            continue;

        // Write the shadow mesh to a .x file
        std::string shadowXName = baseName + "_shadow" + std::to_string(shadowIdx) + ".x";
        std::string shadowXPath = outputDir.pathname() + "/" + shadowXName;
        std::ofstream shadowXOfs(shadowXPath);
        if (!shadowXOfs.is_open())
            continue;

        std::string meshName = writeShadowXFile(shadowXOfs, *child);
        shadowXOfs.close();
        if (meshName.empty())
        {
            std::filesystem::remove(shadowXPath);
            continue;
        }

        // Write a .lod file referencing the .x mesh
        std::string shadowLodName = baseName + "_shadow" + std::to_string(shadowIdx) + ".lod";
        std::string shadowLodPath = outputDir.pathname() + "/" + shadowLodName;
        std::ofstream lodOfs(shadowLodPath);
        if (lodOfs.is_open())
        {
            lodOfs << "MESH " << shadowXName << " " << meshName << "\n";
            lodOfs.close();
        }

        // z-offset from the shadow entity's local transform
        double zOffset = child->localTransform().position().z();
        ofs << childName << " " << shadowLodName << " "
            << std::fixed << std::setprecision(6) << zOffset << "\n";

        ++shadowIdx;
    }

    // Write ANIMATION lines
    // Format: ANIMATION <planName> <xFileName> <animSetName> <fps>
    for (const auto& anim : animations)
    {
        ofs << "ANIMATION " << anim.planName
            << " " << anim.xFileName
            << " " << anim.animSetName
            << " " << std::fixed << std::setprecision(1) << anim.fps << "\n";
    }

    // Write ANIMATION_DATA block (contains CYCLE_TEXTURE, UV_TRANSLATE, COLOUR_PULSE, GENERIC_LIGHT)
    writeAnimationDataBlock(ofs, composite);
}

void MachPhysModelExporter::exportComposite(
    const W4dComposite& composite,
    const SysPathName& outputDir,
    const std::string& name)
{
    W4dLOD nLods = maxLODCount(composite);
    if (nLods == 0)
    {
        std::cerr << "ModelExporter: Composite " << name << " has no meshes" << std::endl;
        return;
    }

    std::vector<std::string> xFileNames;
    std::vector<double> distances;

    for (W4dLOD lod = 0; lod < nLods; ++lod)
    {
        // Use the original mesh filename from models.bin so that override
        // lookups (keyed by RenMesh::pathName_) find the exported files.
        std::string lodBaseName = originalMeshBaseName(composite, lod);
        if (lodBaseName.empty())
            lodBaseName = (lod == 0) ? name : (name + "_lod" + std::to_string(lod));

        std::string xFileName = writeXFile(composite, outputDir, lodBaseName, lod);
        xFileNames.push_back(xFileName);

        // Also export glTF alongside .x
        writeGltfFile(composite, outputDir, lodBaseName, lod);

        // Get the LOD distance from the composite itself (or first link that has this LOD)
        double dist = HUGE_VAL;
        if (composite.hasMesh(lod))
        {
            dist = composite.meshDistance(lod);
        }
        else
        {
            for (const W4dLink* link : composite.links())
            {
                if (link && link->hasMesh(lod))
                {
                    dist = link->meshDistance(lod);
                    break;
                }
            }
        }
        distances.push_back(dist);
    }

    // Check we have at least one valid .x file
    bool anyValid = false;
    for (const auto& fn : xFileNames)
    {
        if (!fn.empty())
        {
            anyValid = true;
            break;
        }
    }

    if (!anyValid)
    {
        std::cerr << "ModelExporter: Composite " << name << " produced no .x files" << std::endl;
        return;
    }

    // Export animations
    auto animations = writeAnimationXFiles(composite, outputDir, name);

    writeCdfFile(composite, outputDir, name, xFileNames, distances, animations);

    std::cout << "ModelExporter: Exported " << name << " ("
              << composite.links().size() << " links, "
              << nLods << " LODs, "
              << animations.size() << " anims)" << std::endl;
}

std::vector<MachPhysModelExporter::AnimInfo> MachPhysModelExporter::writeAnimationXFiles(
    const W4dComposite& composite,
    const SysPathName& outputDir,
    const std::string& baseName)
{
    std::vector<AnimInfo> result;

    // Get all plan names
    ctl_list<std::string> planNames;
    composite.listCompositePlans(&planNames);
    if (planNames.empty())
        return result;

    // Collect link id->name mapping
    std::map<W4dLinkId, std::string> linkIdToName;
    for (const W4dLink* link : composite.links())
    {
        if (link && !link->name().empty())
            linkIdToName[link->id()] = link->name();
    }

    constexpr double sampleFps = 30.0;

    int planIdx = 0;
    for (const auto& planName : planNames)
    {
        W4dCompositePlanPtr planPtr;
        if (!composite.findCompositePlan(planName, &planPtr))
            continue;

        std::string animSetName = planName;
        std::string xFileName = baseName + "_anim" + std::to_string(planIdx) + ".x";
        std::string fullPath = outputDir.pathname() + "/" + xFileName;

        std::ofstream ofs(fullPath);
        if (!ofs.is_open())
            continue;

        ofs << "xof 0303txt 0032\n\n";
        ofs << "AnimationSet " << animSetName << " {\n";

        // Write animations for each link entry in the plan
        const auto& entries = planPtr->entries();
        for (const W4dCompositePlanEntry* entry : entries)
        {
            W4dLinkId linkId = entry->id();
            const W4dEntityPlan& entityPlan = entry->plan();

            if (!entityPlan.hasMotionPlan())
                continue;

            // Find link name
            std::string linkName;
            auto nameIt = linkIdToName.find(linkId);
            if (nameIt != linkIdToName.end())
                linkName = nameIt->second;
            else
                linkName = "link_" + std::to_string(linkId);

            // Sample the motion plan at sampleFps
            PhysRelativeTime duration = entityPlan.endTime();
            if (duration <= 0.0)
                continue;

            int nFrames = static_cast<int>(std::ceil(duration * sampleFps)) + 1;
            if (nFrames < 2)
                nFrames = 2;

            // Collect keyframes
            std::vector<std::pair<int, MexTransform3d>> keyframes;
            for (int f = 0; f < nFrames; ++f)
            {
                double t = (f * duration) / (nFrames - 1);
                MexTransform3d xform;
                uint nObsolete = 0;
                auto state = entityPlan.transform(PhysAbsoluteTime(t), &xform, &nObsolete);
                if (state == W4dEntityPlan::DEFINED)
                {
                    keyframes.push_back({f, xform});
                }
            }

            if (keyframes.empty())
                continue;

            ofs << "  Animation anim_" << linkName << " {\n";
            ofs << "    { " << linkName << " }\n";

            // Write orientation keys (type 0: quaternion)
            ofs << "    AnimationKey {\n";
            ofs << "      0\n";
            ofs << "      " << keyframes.size() << "\n";
            for (const auto& [frameId, xform] : keyframes)
            {
                MexQuaternion q = xform.rotationAsQuaternion();
                const MexVec3& v = q.vector();
                MATHEX_SCALAR s = q.scalar();

                // .x format quaternion is (s, -vx, -vy, -vz) -- negate vector to match
                // the convention used by the parser (which negates on read)
                ofs << "      " << frameId << "; 4; "
                    << std::fixed << std::setprecision(6)
                    << s << ", " << -v.x() << ", " << -v.y() << ", " << -v.z() << ";;\n";
            }
            ofs << "    }\n";

            // Write location keys (type 2: position)
            ofs << "    AnimationKey {\n";
            ofs << "      2\n";
            ofs << "      " << keyframes.size() << "\n";
            for (const auto& [frameId, xform] : keyframes)
            {
                MexPoint3d pos = xform.position();
                ofs << "      " << frameId << "; 3; "
                    << std::fixed << std::setprecision(6)
                    << pos.x() << ", " << pos.y() << ", " << pos.z() << ";;\n";
            }
            ofs << "    }\n";

            ofs << "  }\n"; // end Animation
        }

        ofs << "}\n"; // end AnimationSet
        ofs.close();

        AnimInfo info;
        info.planName = planName;
        info.xFileName = xFileName;
        info.animSetName = animSetName;
        info.fps = sampleFps;
        result.push_back(info);

        ++planIdx;
    }

    return result;
}

// Static reverse lookup: CDF filename (e.g. "tet3.cdf") -> full path (e.g. "models/technici/techboy/level3/tet3.cdf").
// Built from all compositeFileName() paths hardcoded in the machphys source.
// Uses multimap because some basenames are ambiguous (chasm.cdf, hold.cdf).
static const std::unordered_multimap<std::string, std::string>& cdfPathLookup()
{
    static const std::unordered_multimap<std::string, std::string> table = {
        // Administrators
        {"adb1.cdf", "models/administ/boss/level1/adb1.cdf"},
        {"adb2.cdf", "models/administ/boss/level2/adb2.cdf"},
        {"adc4.cdf", "models/administ/command/level4/adc4.cdf"},
        {"adc5.cdf", "models/administ/command/level5/adc5.cdf"},
        {"ado2.cdf", "models/administ/overseer/level2/ado2.cdf"},
        {"ado3.cdf", "models/administ/overseer/level3/ado3.cdf"},
        // Aggressors
        {"aga2.cdf", "models/aggresso/assassin/level2/aga2.cdf"},
        {"aga3.cdf", "models/aggresso/assassin/level3/aga3.cdf"},
        {"agb3.cdf", "models/aggresso/ballista/level3/agb3.cdf"},
        {"agb4.cdf", "models/aggresso/ballista/level4/agb4.cdf"},
        {"agg1.cdf", "models/aggresso/grunt/level1/agg1.cdf"},
        {"agg2.cdf", "models/aggresso/grunt/level2/agg2.cdf"},
        {"agg3.cdf", "models/aggresso/grunt/level3/agg3.cdf"},
        {"agk3.cdf", "models/aggresso/knight/level3/agk3.cdf"},
        {"agk4.cdf", "models/aggresso/knight/level4/agk4.cdf"},
        {"agk5.cdf", "models/aggresso/knight/level5/agk5.cdf"},
        {"agm5.cdf", "models/aggresso/ninja/level5/agm5.cdf"},
        // Constructors
        {"cod1.cdf", "models/construc/dozer/level1/cod1.cdf"},
        {"cob3.cdf", "models/construc/builder/level3/cob3.cdf"},
        {"como4.cdf", "models/construc/behemoth/level4/como4.cdf"},
        {"com5.cdf", "models/construc/behemoth/level5/com5.cdf"},
        {"sparks.cdf", "models/construc/sparks/sparks.cdf"},
        {"torch.cdf", "models/construc/torch/torch.cdf"},
        // Technicians
        {"tel2.cdf", "models/technici/labtech/level2/tel2.cdf"},
        {"tet3.cdf", "models/technici/techboy/level3/tet3.cdf"},
        {"teb5.cdf", "models/technici/brainbox/level5/teb5.cdf"},
        // Locators
        {"log1.cdf", "models/locator/geo/level1/log1.cdf"},
        {"log2.cdf", "models/locator/geo/level2/log2.cdf"},
        {"log3.cdf", "models/locator/geo/level3/log3.cdf"},
        {"los3.cdf", "models/locator/spy/level3/los3.cdf"},
        {"los5.cdf", "models/locator/spy/level5/los5.cdf"},
        // Transporters
        {"trp2.cdf", "models/transpor/person/level2/trp2.cdf"},
        {"trp4.cdf", "models/transpor/person/level4/trp4.cdf"},
        {"trp5.cdf", "models/transpor/person/level5/trp5.cdf"},
        {"trr1.cdf", "models/transpor/resource/level1/trr1.cdf"},
        {"trr2.cdf", "models/transpor/resource/level2/trr2.cdf"},
        {"trr3.cdf", "models/transpor/resource/level3/trr3.cdf"},
        {"trr4.cdf", "models/transpor/resource/level4/trr4.cdf"},
        {"trr5.cdf", "models/transpor/resource/level5/trr5.cdf"},
        // Shared
        {"fplat.cdf", "models/shared/faceplat/fplat.cdf"},
        {"xhvg.cdf", "models/shared/hover/typeG/xhvg.cdf"},
        {"xhvj.cdf", "models/shared/hover/typeJ/xhvj.cdf"},
        {"xtra.cdf", "models/shared/tracks/typeA/xtra.cdf"},
        {"xtrb.cdf", "models/shared/tracks/typeB/xtrb.cdf"},
        {"xtrf.cdf", "models/shared/tracks/typeF/xtrf.cdf"},
        {"xtrg.cdf", "models/shared/tracks/typeG/xtrg.cdf"},
        {"xtrh.cdf", "models/shared/tracks/typeH/xtrh.cdf"},
        {"xtri.cdf", "models/shared/tracks/typeI/xtri.cdf"},
        {"xtrj.cdf", "models/shared/tracks/typeJ/xtrj.cdf"},
        {"xtrk.cdf", "models/shared/tracks/typeK/xtrk.cdf"},
        {"xwhl.cdf", "models/shared/wheel/xwhl.cdf"},
        // Factory
        {"fac1e.cdf", "models/factory/civilian/level1/exterior/fac1e.cdf"},
        {"fac3e.cdf", "models/factory/civilian/level3/exterior/fac3e.cdf"},
        {"fac5e.cdf", "models/factory/civilian/level5/exterior/fac5e.cdf"},
        {"fam1e.cdf", "models/factory/military/level1/exterior/fam1e.cdf"},
        {"fam3e.cdf", "models/factory/military/level3/exterior/fam3e.cdf"},
        {"fam4e.cdf", "models/factory/military/level4/exterior/fam4e.cdf"},
        {"fam5e.cdf", "models/factory/military/level5/exterior/fam5e.cdf"},
        // Labs
        {"lac1e.cdf", "models/lab/civilian/level1/exterior/lac1e.cdf"},
        {"lac1i.cdf", "models/lab/civilian/level1/interior/lac1i.cdf"},
        {"lac1w.cdf", "models/lab/civilian/level1/wirefram/lac1w.cdf"},
        {"lac3e.cdf", "models/lab/civilian/level3/exterior/lac3e.cdf"},
        {"lac3i.cdf", "models/lab/civilian/level3/interior/lac3i.cdf"},
        {"lac3w.cdf", "models/lab/civilian/level3/wirefram/lac3w.cdf"},
        {"lam1e.cdf", "models/lab/military/level1/exterior/lam1e.cdf"},
        {"lam1i.cdf", "models/lab/military/level1/interior/lam1i.cdf"},
        {"lam1w.cdf", "models/lab/military/level1/wirefram/lam1w.cdf"},
        {"lam3e.cdf", "models/lab/military/level3/exterior/lam3e.cdf"},
        {"lam3i.cdf", "models/lab/military/level3/interior/lam3i.cdf"},
        {"lam3w.cdf", "models/lab/military/level3/wirefram/lam3w.cdf"},
        // Weapons Lab
        {"sw1e.cdf", "models/softlab/level1/exterior/sw1e.cdf"},
        {"sw1i.cdf", "models/softlab/level1/interior/sw1i.cdf"},
        {"sw1w.cdf", "models/softlab/level1/wirefram/sw1w.cdf"},
        // Smelter
        {"sm1e.cdf", "models/smelter/level1/exterior/sm1e.cdf"},
        {"sm1i.cdf", "models/smelter/level1/interior/sm1i.cdf"},
        {"sm1w.cdf", "models/smelter/level1/wirefram/sm1w.cdf"},
        {"sm3e.cdf", "models/smelter/level3/exterior/sm3e.cdf"},
        {"sm3i.cdf", "models/smelter/level3/interior/sm3i.cdf"},
        {"sm3w.cdf", "models/smelter/level3/wirefram/sm3w.cdf"},
        // Mine
        {"mn1e.cdf", "models/mine/level1/exterior/mn1e.cdf"},
        {"mn1w.cdf", "models/mine/level1/wirefram/mn1w.cdf"},
        {"mn3e.cdf", "models/mine/level3/exterior/mn3e.cdf"},
        {"mn3w.cdf", "models/mine/level3/wirefram/mn3w.cdf"},
        {"mn5e.cdf", "models/mine/level5/exterior/mn5e.cdf"},
        {"mn5w.cdf", "models/mine/level5/wirefram/mn5w.cdf"},
        // Garrison
        {"ga1e.cdf", "models/garrison/level1/exterior/ga1e.cdf"},
        {"ga1i.cdf", "models/garrison/level1/interior/ga1i.cdf"},
        {"ga1w.cdf", "models/garrison/level1/wirefram/ga1w.cdf"},
        // Beacon
        {"bk1e.cdf", "models/beacon/level1/exterior/bk1e.cdf"},
        {"bk1w.cdf", "models/beacon/level1/wirefram/bk1w.cdf"},
        {"bk2e.cdf", "models/beacon/level2/exterior/bk2e.cdf"},
        {"bk2w.cdf", "models/beacon/level2/wirefram/bk2w.cdf"},
        {"bk3.cdf", "models/beacon/level3/bk3.cdf"},
        {"bk3e.cdf", "models/beacon/level3/exterior/bk3e.cdf"},
        {"bk3w.cdf", "models/beacon/level3/wirefram/bk3w.cdf"},
        // Missile Emplacement
        {"met1e.cdf", "models/missilee/turret/level1/exterior/met1e.cdf"},
        {"met1w.cdf", "models/missilee/turret/level1/wirefram/met1w.cdf"},
        {"met2e.cdf", "models/missilee/turret/level2/exterior/met2e.cdf"},
        {"met2w.cdf", "models/missilee/turret/level2/wirefram/met2w.cdf"},
        {"met3e.cdf", "models/missilee/turret/level3/exterior/met3e.cdf"},
        {"met3w.cdf", "models/missilee/turret/level3/wirefram/met3w.cdf"},
        {"mes3e.cdf", "models/missilee/sentry/level3/exterior/mes3e.cdf"},
        {"mes3w.cdf", "models/missilee/sentry/level3/wirefram/mes3w.cdf"},
        {"mes4e.cdf", "models/missilee/sentry/level4/exterior/mes4e.cdf"},
        {"mes4w.cdf", "models/missilee/sentry/level4/wirefram/mes4w.cdf"},
        {"mel4e.cdf", "models/missilee/launcher/level4/exterior/mel4e.cdf"},
        {"mel4w.cdf", "models/missilee/launcher/level4/wirefram/mel4w.cdf"},
        {"mei5e.cdf", "models/missilee/icbm/level5/exterior/mei5e.cdf"},
        // Pod
        {"pd1e.cdf", "models/pod/level1/exterior/pd1e.cdf"},
        {"pd1i.cdf", "models/pod/level1/interior/pd1i.cdf"},
        // Holograph
        {"holo.cdf", "models/holo/level1/holo.cdf"},
        // Demolition
        {"demolish.cdf", "models/destroy/demolish/demolish.cdf"},
        // Weapons
        {"auto.cdf", "models/weapons/auto/auto.cdf"},
        {"bomb.cdf", "models/weapons/bee/bomb.cdf"},
        {"chasm.cdf", "models/weapons/bee/chasm.cdf"},
        {"chasm.cdf", "models/weapons/punch/chasm.cdf"},
        {"hold.cdf", "models/weapons/bee/hold.cdf"},
        {"hold.cdf", "models/weapons/lmissile/hold.cdf"},
        {"bltr.cdf", "models/weapons/bolter/bltr.cdf"},
        {"chargea.cdf", "models/weapons/chargea/chargea.cdf"},
        {"charges.cdf", "models/weapons/charges/charges.cdf"},
        {"elecch.cdf", "models/weapons/elecch/elecch.cdf"},
        {"flameth1.cdf", "models/weapons/flameth1/flameth1.cdf"},
        {"flameth2.cdf", "models/weapons/flameth2/flameth2.cdf"},
        {"hvyb1.cdf", "models/weapons/heavyb1/hvyb1.cdf"},
        {"hvyb2.cdf", "models/weapons/heavyb2/hvyb2.cdf"},
        {"beam.cdf", "models/weapons/ioncann/beam.cdf"},
        {"lmiss.cdf", "models/weapons/lmissile/lmiss.cdf"},
        {"mis1.cdf", "models/weapons/missile/level1/mis1.cdf"},
        {"mis2.cdf", "models/weapons/missile/level2/mis2.cdf"},
        {"mis3.cdf", "models/weapons/missile/level3/mis3.cdf"},
        {"mis4.cdf", "models/weapons/missile/level4/mis4.cdf"},
        {"mis5.cdf", "models/weapons/missile/level5/mis5.cdf"},
        {"mis6.cdf", "models/weapons/missile/level6/mis6.cdf"},
        {"mis7.cdf", "models/weapons/missile/level7/mis7.cdf"},
        {"mult1.cdf", "models/weapons/mult1/mult1.cdf"},
        {"mult2.cdf", "models/weapons/mult2/mult2.cdf"},
        {"mult3.cdf", "models/weapons/mult3/mult3.cdf"},
        {"mult4.cdf", "models/weapons/mult4/mult4.cdf"},
        {"mult5.cdf", "models/weapons/mult5/mult5.cdf"},
        {"mult6.cdf", "models/weapons/mult6/mult6.cdf"},
        {"mult7.cdf", "models/weapons/mult7/mult7.cdf"},
        {"nmisea.cdf", "models/weapons/nmissile/nmisea.cdf"},
        {"point.cdf", "models/weapons/nmissile/point.cdf"},
        {"plabolt2.cdf", "models/weapons/plabolt2/plabolt2.cdf"},
        {"plaboltc.cdf", "models/weapons/plaboltc/plaboltc.cdf"},
        {"plaboltr.cdf", "models/weapons/plaboltr/plaboltr.cdf"},
        {"plasmc1.cdf", "models/weapons/plasmc1/plasmc1.cdf"},
        {"plasmc2.cdf", "models/weapons/plasmc2/plasmc2.cdf"},
        {"plasmr.cdf", "models/weapons/plasmr/plasmr.cdf"},
        {"pulboltc.cdf", "models/weapons/pulboltc/pulboltc.cdf"},
        {"pulboltr.cdf", "models/weapons/pulboltr/pulboltr.cdf"},
        {"pulsec.cdf", "models/weapons/pulsec/pulsec.cdf"},
        {"pulsr.cdf", "models/weapons/pulser/pulsr.cdf"},
        {"treachry.cdf", "models/weapons/treachry/treachry.cdf"},
        {"virusgun.cdf", "models/weapons/virusgun/virusgun.cdf"},
        {"vrtx.cdf", "models/weapons/vortex/vrtx.cdf"},
        {"bullet.cdf", "models/weapons/wasp/light/bullet.cdf"},
        {"lightgun.cdf", "models/weapons/wasp/light/lightgun.cdf"},
        {"splat.cdf", "models/weapons/wasp/light/splat.cdf"},
        {"metalgun.cdf", "models/weapons/wasp/metal/metalgun.cdf"},
        {"missile.cdf", "models/weapons/wasp/metal/missile.cdf"},
    };
    return table;
}

// Split a full CDF path into (subDir, baseName) stripping the .cdf extension.
static std::pair<std::string, std::string> splitCdfPath(const std::string& fullName)
{
    std::string stripped = fullName;
    auto dotPos = stripped.rfind('.');
    if (dotPos != std::string::npos)
        stripped = stripped.substr(0, dotPos);

    std::string subDir;
    std::string baseName;
    auto slashPos = stripped.rfind('/');
    if (slashPos != std::string::npos)
    {
        subDir = stripped.substr(0, slashPos);
        baseName = stripped.substr(slashPos + 1);
    }
    else
    {
        baseName = stripped;
    }

    return {subDir, baseName};
}

// Resolve a comp.name() (which may be just a filename like "tet3.cdf" or a full path)
// into one or more (subDir, baseName) pairs for directory hierarchy reconstruction.
// Checks the authoritative pathMap first, then falls back to the static lookup table.
// Returns multiple results for ambiguous basenames (e.g. hold.cdf -> bee + lmissile).
static std::vector<std::pair<std::string, std::string>> resolveCdfPaths(
    const std::string& compName, const CdfPathMap& pathMap)
{
    std::string fullName = compName;

    // If name contains a slash, it's already a full path
    if (fullName.find('/') != std::string::npos)
        return {splitCdfPath(fullName)};

    // Try the authoritative pathMap first
    auto it = pathMap.find(fullName);
    if (it != pathMap.end())
        return {splitCdfPath(it->second)};

    // Fall back to the static reverse lookup table (multimap: may have multiple entries)
    auto range = cdfPathLookup().equal_range(fullName);
    std::vector<std::pair<std::string, std::string>> results;
    for (auto it2 = range.first; it2 != range.second; ++it2)
        results.push_back(splitCdfPath(it2->second));

    if (results.empty())
        results.push_back(splitCdfPath(fullName));

    return results;
}

// Helper: export a single composite using its name() for path resolution.
// For ambiguous basenames, exports to all matching paths.
static void exportNamedComposite(
    const W4dComposite& comp,
    const SysPathName& outputDir,
    const std::string& fallbackName,
    const CdfPathMap& pathMap)
{
    std::string compName = comp.name();
    if (compName.empty())
    {
        MachPhysModelExporter::exportComposite(comp, outputDir, fallbackName);
        return;
    }

    auto resolved = resolveCdfPaths(compName, pathMap);

    for (const auto& [subDir, baseName] : resolved)
    {
        std::string modelDir = outputDir.pathname();
        if (!subDir.empty())
        {
            modelDir += "/" + subDir;
            std::filesystem::create_directories(modelDir);
        }

        MachPhysModelExporter::exportComposite(comp, SysPathName(modelDir), baseName);
    }
}

// Export a standalone LOD entity (non-composite with meshes) by writing
// .x files for each LOD and a .lod file referencing them.
// The output directory is derived from the mesh's pathName (e.g.
// "models/debris/debris.x" -> outputDir/models/debris/).
// The .lod filename comes from entity.name() (set by readLODFile and persisted).
static void exportStandaloneEntity(
    const W4dEntity& ent,
    const SysPathName& outputDir)
{
    uint nLods = ent.nLODs();
    if (nLods == 0 || !ent.hasMesh(0))
    {
        std::cout << "[EXPORT] LOD skip (no mesh): name='" << ent.name() << "' nLods=" << nLods << std::endl;
        return;
    }

    // Derive output directory from the first LOD mesh's pathName
    const RenMeshInstance& firstMesh = ent.mesh(0);
    Ren::ConstMeshPtr meshPtr = firstMesh.mesh();
    // e.g. "models/debris/debris.x"
    std::string meshPath = meshPtr->pathName().set() ? meshPtr->pathName().pathname() : std::string();
    std::cout << "[EXPORT] LOD entity: name='" << ent.name() << "' meshPath='" << meshPath
              << "' meshName='" << meshPtr->meshName() << "' nLods=" << nLods << std::endl;
    if (meshPath.empty())
        return;

    auto lastSlash = meshPath.rfind('/');
    std::string relDir = (lastSlash != std::string::npos) ? meshPath.substr(0, lastSlash) : "";

    // Create output directory
    std::string outDir = outputDir.pathname();
    if (!relDir.empty())
        outDir += "/" + relDir;
    std::filesystem::create_directories(outDir);

    // The .lod base name comes from entity.name().
    // W4dGeneric::loadLODFile strips the extension, so name() is e.g. "bolter" not
    // "bolter.lod".  W4dEntity::readLODFile keeps it as "debris.lod".
    // Normalise: use name as base, always produce baseName.lod.
    std::string lodBase = ent.name();
    if (lodBase.empty())
    {
        std::string xLeaf = (lastSlash != std::string::npos) ? meshPath.substr(lastSlash + 1) : meshPath;
        auto dotPos = xLeaf.rfind('.');
        lodBase = (dotPos != std::string::npos) ? xLeaf.substr(0, dotPos) : xLeaf;
    }
    // Strip .lod extension if already present (from readLODFile path)
    if (lodBase.size() > 4 && lodBase.substr(lodBase.size() - 4) == ".lod")
        lodBase = lodBase.substr(0, lodBase.size() - 4);

    std::string lodName = lodBase + ".lod";

    std::string lodPath = outDir + "/" + lodName;
    std::ofstream lodFile(lodPath);
    if (!lodFile.is_open())
    {
        std::cerr << "[EXPORT] Failed to create LOD file: " << lodPath << std::endl;
        return;
    }

    for (uint lod = 0; lod < nLods; ++lod)
    {
        if (!ent.hasMesh(lod))
        {
            lodFile << "EMPTY_MESH\n";
        }
        else
        {
            const RenMeshInstance& mi = ent.mesh(lod);
            Ren::ConstMeshPtr mp = mi.mesh();
            std::string mName = mp->meshName();

            // Determine the .x filename for this LOD
            std::string xName;
            if (lod == 0)
                xName = lodBase + ".x";
            else
                xName = lodBase + "_lod" + std::to_string(lod) + ".x";

            // Write the .x file
            std::string xPath = outDir + "/" + xName;
            std::ofstream xFile(xPath);
            if (xFile.is_open())
            {
                xFile << "xof 0302txt 0064\n\n";
                xFile << "Header {\n 1;\n 0;\n 1;\n}\n\n";
                MachPhysModelExporter::writeMesh(xFile, mi, "");
                xFile.close();
                std::cout << "[EXPORT] Wrote " << xPath << std::endl;
            }

            lodFile << "MESH " << xName << " " << mName << "\n";
        }

        // Write DISTANCE line for non-final LODs (last LOD has HUGE_VAL which
        // cannot be converted from MexDouble).
        if (lod + 1 < nLods)
        {
            double dist = static_cast<double>(ent.meshDistance(lod));
            lodFile << "DISTANCE " << dist << "\n";
        }
    }

    lodFile.close();
    std::cout << "[EXPORT] Wrote " << lodPath << std::endl;
}

// Recursively walk the entity tree and export every W4dComposite that has a
// name (i.e. was loaded from a CDF).  This catches interior models which are
// W4dGenericComposite objects nested under W4dDomain children of constructions.
// Also exports standalone LOD entities (non-composites with meshes).
static void exportCompositesRecursive(
    const W4dEntity& entity,
    const SysPathName& outputDir,
    const std::string& prefix,
    const CdfPathMap& pathMap,
    int& idx)
{
    for (const W4dEntity* child : entity.children())
    {
        if (!child)
            continue;

        if (child->isComposite())
        {
            const W4dComposite& comp = child->asComposite();

            // Some composites (e.g. MachPhysElectro via MachPhysLinearProjectile)
            // load meshes via readLODFile, not readCompositeFile.  These have
            // direct meshes on the composite entity itself.  Export them as
            // standalone LOD entities as well as trying the CDF path.
            if (child->hasMesh())
                exportStandaloneEntity(*child, outputDir);

            std::string fallback = prefix + "_" + std::to_string(idx);
            exportNamedComposite(comp, outputDir, fallback, pathMap);
            ++idx;

            // Also recurse into this composite's non-link children
            // (links are part of the composite and already exported with it).
            exportCompositesRecursive(comp, outputDir, prefix, pathMap, idx);
        }
        else
        {
            // Export standalone LOD entities (debris, effects, etc.)
            if (child->hasMesh())
                exportStandaloneEntity(*child, outputDir);

            // Non-composite entities (e.g. W4dDomain) may still have
            // composite children deeper in the tree.
            exportCompositesRecursive(*child, outputDir, prefix, pathMap, idx);
        }
    }
}

void MachPhysExportFromRoot(
    const W4dRoot& root,
    const SysPathName& outputDir,
    const std::string& prefix,
    const CdfPathMap& pathMap)
{
    int idx = 0;
    exportCompositesRecursive(root, outputDir, prefix, pathMap, idx);
}

// Collect all RenMesh instances from a composite and its links (all LODs).
// Appends (meshPathName, meshName, RenMeshInstance*) to the output vector.
static void collectMeshesFromComposite(
    const W4dComposite& comp,
    std::vector<std::tuple<std::string, std::string, const RenMeshInstance*>>& out)
{
    // Helper: collect from one entity (composite root or link)
    auto collect = [&](const W4dEntity& ent)
    {
        for (W4dLOD lod = 0; ent.hasMesh(lod); ++lod)
        {
            const RenMeshInstance& mi = ent.mesh(lod);
            Ren::ConstMeshPtr mp = mi.mesh();
            if (!mp.isDefined())
                continue;
            const std::string& pn = mp->pathName().pathname();
            if (!pn.empty())
                out.emplace_back(pn, mp->meshName(), &mi);
        }
    };

    collect(comp);
    for (const W4dLink* link : comp.links())
    {
        if (link)
            collect(*link);
    }
}

// Recursively collect meshes from all composites under an entity tree.
static void collectMeshesRecursive(
    const W4dEntity& entity,
    std::vector<std::tuple<std::string, std::string, const RenMeshInstance*>>& out)
{
    for (const W4dEntity* child : entity.children())
    {
        if (!child)
            continue;
        if (child->isComposite())
        {
            collectMeshesFromComposite(child->asComposite(), out);
            collectMeshesRecursive(child->asComposite(), out);
        }
        else
        {
            collectMeshesRecursive(*child, out);
        }
    }
}

// Normalize Windows backslashes to forward slashes.
static std::string normalizePath(const std::string& p)
{
    std::string r = p;
    for (char& c : r)
        if (c == '\\')
            c = '/';
    return r;
}

void MachPhysModelExporter::exportSharedModels(const SysPathName& outputDir, const std::vector<const W4dRoot*>& roots)
{
    // Table: CDF path -> expected .x basename (without extension).
    // Derived from preloadMachines() CDF paths and `strings models.bin`.
    struct SharedEntry
    {
        std::string cdfPath;
        std::string xBaseName;
    };
    const std::vector<SharedEntry> sharedTable = {
        {"models/shared/faceplat/fplat.cdf", "fplat20"},
        {"models/shared/hover/typeG/xhvg.cdf", "xhvg04"},
        {"models/shared/hover/typeJ/xhvj.cdf", "xhvj04"},
        {"models/shared/tracks/typeA/xtra.cdf", "xtra"},
        {"models/shared/tracks/typeB/xtrb.cdf", "xtrb"},
        {"models/shared/tracks/typeF/xtrf.cdf", "xtrf"},
        {"models/shared/tracks/typeG/xtrg.cdf", "xtrg"},
        {"models/shared/tracks/typeH/xtrh.cdf", "xtrh01"},
        {"models/shared/tracks/typeI/xtri.cdf", "xtri"},
        {"models/shared/tracks/typeJ/xtrj.cdf", "xtrj"},
        {"models/shared/tracks/typeK/xtrk.cdf", "xtrk"},
        {"models/shared/wheel/xwhl.cdf", "xwhl48"},
    };

    // Collect all meshes from all provided factory roots.
    std::vector<std::tuple<std::string, std::string, const RenMeshInstance*>> allMeshes;
    for (const W4dRoot* root : roots)
    {
        if (root)
            collectMeshesRecursive(*root, allMeshes);
    }

    // Build map: normalized .x path -> vector of (meshName, RenMeshInstance*)
    // Only include meshes whose path contains "shared".
    std::map<std::string, std::vector<std::pair<std::string, const RenMeshInstance*>>> xFileMap;
    for (const auto& [path, name, mi] : allMeshes)
    {
        std::string np = normalizePath(path);
        if (np.find("shared") != std::string::npos)
        {
            // Deduplicate by mesh name within the same .x file
            auto& vec = xFileMap[np];
            bool found = false;
            for (const auto& [n, _] : vec)
            {
                if (n == name)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                vec.emplace_back(name, mi);
        }
    }

    std::cout << "ModelExporter: Found " << xFileMap.size() << " shared .x files in machine exemplars." << std::endl;

    // For each shared table entry, write the .x file and .cdf wrapper.
    for (const auto& entry : sharedTable)
    {
        // Derive the expected .x path from the CDF path
        auto slashPos = entry.cdfPath.rfind('/');
        std::string dir = (slashPos != std::string::npos) ? entry.cdfPath.substr(0, slashPos) : "";
        std::string xPath = dir + "/" + entry.xBaseName + ".x";

        // Create output directory
        std::string outDir = outputDir.pathname() + "/" + dir;
        std::filesystem::create_directories(outDir);

        auto it = xFileMap.find(xPath);
        if (it != xFileMap.end() && !it->second.empty())
        {
            // Write the .x file
            std::string xFullPath = outputDir.pathname() + "/" + xPath;
            std::ofstream xfs(xFullPath);
            if (xfs.is_open())
            {
                xfs << "xof 0303txt 0032\n\n";

                // Single root frame to avoid $dummy_root wrapping
                xfs << "Frame " << entry.xBaseName << " {\n";
                xfs << "  FrameTransformMatrix {\n";
                xfs << "    1.000000, 0.000000, 0.000000, 0.000000,\n";
                xfs << "    0.000000, 1.000000, 0.000000, 0.000000,\n";
                xfs << "    0.000000, 0.000000, 1.000000, 0.000000,\n";
                xfs << "    0.000000, 0.000000, 0.000000, 1.000000;;\n";
                xfs << "  }\n";

                for (const auto& [meshName, mi] : it->second)
                {
                    xfs << "  Frame " << meshName << " {\n";
                    writeMesh(xfs, *mi, "    ");
                    xfs << "  }\n\n";
                }

                xfs << "}\n";

                xfs.close();
                std::cout << "ModelExporter: Wrote shared " << xPath << " (" << it->second.size() << " meshes)" << std::endl;
            }

            // Also export glTF alongside .x
            writeSharedGltfFile(SysPathName(outDir), entry.xBaseName, it->second);
        }
        else
        {
            // No mesh data found (e.g., typeF/typeG tracks) -- write an empty .x stub
            std::string xFullPath = outputDir.pathname() + "/" + xPath;
            std::ofstream xfs(xFullPath);
            if (xfs.is_open())
            {
                xfs << "xof 0303txt 0032\n\n";
                xfs << "Frame empty {\n}\n";
                xfs.close();
                std::cout << "ModelExporter: Wrote empty stub " << xPath << std::endl;
            }
        }

        // Write the .cdf file
        std::string cdfFullPath = outputDir.pathname() + "/" + entry.cdfPath;
        std::ofstream cfs(cdfFullPath);
        if (cfs.is_open())
        {
            cfs << "MODEL " << entry.xBaseName << "\n";
            cfs.close();
        }
    }
}

void MachPhysModelExporter::exportTerrainTiles(
    const MachPhysPlanetSurface& surface,
    const SysPathName& outputDir)
{
    std::set<std::string> exported;

    for (size_t y = 0; y < surface.nYTiles(); ++y)
    {
        for (size_t x = 0; x < surface.nXTiles(); ++x)
        {
            const MachPhysTerrainTile& tile = surface.tile(x, y);
            std::string lodPath = tile.lodFileName().pathname();

            // Skip duplicates (many tiles share the same .lod)
            if (exported.count(lodPath))
                continue;
            exported.insert(lodPath);

            // lodPath is e.g. "models/planet/m_desert/lod/up.lod"
            // Split into directory + filename
            auto lastSlash = lodPath.rfind('/');
            std::string relDir = (lastSlash != std::string::npos) ? lodPath.substr(0, lastSlash) : "";
            std::string lodLeaf = (lastSlash != std::string::npos) ? lodPath.substr(lastSlash + 1) : lodPath;

            // Derive base name (strip .lod extension)
            std::string lodBase = lodLeaf;
            auto dotPos = lodBase.rfind('.');
            if (dotPos != std::string::npos)
                lodBase = lodBase.substr(0, dotPos);

            // Create output directory
            std::string outDir = outputDir.pathname();
            if (!relDir.empty())
                outDir += "/" + relDir;
            std::filesystem::create_directories(outDir);

            // Export using exportStandaloneEntity logic but with known path
            const W4dEntity& ent = tile;
            uint nLods = ent.nLODs();
            if (nLods == 0 || !ent.hasMesh(0))
                continue;

            std::string lodFilePath = outDir + "/" + lodLeaf;
            std::ofstream lodFile(lodFilePath);
            if (!lodFile.is_open())
            {
                std::cerr << "[EXPORT] Failed to create terrain LOD file: " << lodFilePath << std::endl;
                continue;
            }

            for (uint lod = 0; lod < nLods; ++lod)
            {
                if (!ent.hasMesh(lod))
                {
                    lodFile << "EMPTY_MESH\n";
                }
                else
                {
                    const RenMeshInstance& mi = ent.mesh(lod);
                    Ren::ConstMeshPtr mp = mi.mesh();
                    std::string mName = mp->meshName();

                    std::string xName;
                    if (lod == 0)
                        xName = lodBase + ".x";
                    else
                        xName = lodBase + "_lod" + std::to_string(lod) + ".x";

                    std::string xPath = outDir + "/" + xName;
                    std::ofstream xFile(xPath);
                    if (xFile.is_open())
                    {
                        xFile << "xof 0302txt 0064\n\n";
                        xFile << "Header {\n 1;\n 0;\n 1;\n}\n\n";
                        writeMesh(xFile, mi, "");
                        xFile.close();
                    }

                    lodFile << "MESH " << xName << " " << mName << "\n";
                }

                if (lod + 1 < nLods)
                {
                    double dist = static_cast<double>(ent.meshDistance(lod));
                    lodFile << "DISTANCE " << dist << "\n";
                }
            }

            lodFile.close();
            std::cout << "[EXPORT] Wrote terrain tile " << lodFilePath << " (" << nLods << " LODs)" << std::endl;
        }
    }

    std::cout << "[EXPORT] Exported " << exported.size() << " unique terrain tiles." << std::endl;
}

void MachPhysModelExporter::exportAll(const SysPathName& outputDir)
{
    std::filesystem::create_directories(outputDir.pathname());

    std::cout << "ModelExporter: Exporting all models to " << outputDir.pathname() << std::endl;

    MachPhysPersistence::instance().exportAllModels(outputDir);

    std::cout << "ModelExporter: Export complete." << std::endl;
}
