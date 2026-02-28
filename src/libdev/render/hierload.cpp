/*
 * H I E R L O A D . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <string>
#include "render/hierload.hpp"

#include "mathex/transf3d.hpp"
#include "mathex/vec3.hpp"
#include "mathex/point3d.hpp"
#include "system/pathname.hpp"
#include "render/hierbuil.hpp"
#include "render/internal/meshid.hpp"
#include "render/render.hpp"
#include "formats_support/IMeshLoader.hpp"
#include "formats_support/MeshData.hpp"

// Convert a HierarchyNode tree into RenHierarchyBuilder::addFrame calls.
// static
void RenHierarchyLoader::buildFrame(const RenI::HierarchyNode& node, RenHierarchyBuilder* builder)
{
    MexVec3 xBasis(node.xBasisX, node.xBasisY, node.xBasisZ);
    MexVec3 yBasis(node.yBasisX, node.yBasisY, node.yBasisZ);
    MexVec3 zBasis(node.zBasisX, node.zBasisY, node.zBasisZ);
    MexPoint3d position(node.posX, node.posY, node.posZ);
    MexTransform3d xform(xBasis, yBasis, zBasis, position);

    size_t nChildren = node.children.size();

    if (!node.meshName.empty())
    {
        SysPathName filePath(node.filePath);
        RenIMeshID meshId(filePath, node.meshName, node.scale);
        builder->addFrame(xform, nChildren, meshId);
    }
    else if (!node.instanceName.empty())
    {
        builder->addFrame(xform, nChildren, node.instanceName);
    }
    else
    {
        builder->addFrame(xform, nChildren);
    }

    for (const auto& child : node.children)
        buildFrame(child, builder);
}

// Note the file should not have an extension
// static
void RenHierarchyLoader::load(const SysPathName& pathName, RenHierarchyBuilder* pBuilder)
{
    PRE(!pathName.hasExtension());

    // Try each registered loader in priority order.
    for (const auto& loader : Ren::meshLoaders())
    {
        for (const auto& ext : loader->supportedExtensions())
        {
            SysPathName candidate(pathName);
            candidate.extension(ext);

            if (candidate.existsAsFile())
            {
                RenI::HierarchyData hierData = loader->loadHierarchy(candidate);
                for (const auto& root : hierData.roots)
                    buildFrame(root, pBuilder);
                return;
            }
        }
    }
}

void RenHierarchyLoader::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const RenHierarchyLoader& t)
{

    o << "RenHierarchyLoader " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "RenHierarchyLoader " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End HIERLOAD.CPP *************************************************/
