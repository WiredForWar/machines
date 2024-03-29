/*
 * H I E R L O A D . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <string>
#include "render/hierload.hpp"

#include "system/pathname.hpp"
#include "render/hierbuil.hpp"
#include "render/internal/meshload.hpp"
#include "render/internal/gxmeshload.hpp"

// Note the file should not have an extension
// static
void RenHierarchyLoader::load(const SysPathName& pathName, RenHierarchyBuilder* pBuilder)
{
    PRE((! pathName.hasExtension()) || (pathName.extension() != "agt") || (pathName.extension() != "x"));

    SysPathName withExtDX(pathName);
    withExtDX.extension("x");
    SysPathName withExtAGT(pathName);
    withExtAGT.extension("agt");

    ASSERT(withExtDX.existsAsFile() || withExtAGT.existsAsFile(), "");

    if (withExtDX.existsAsFile())
    {
        RenID3DMeshLoader::instance().load(withExtDX, pBuilder);
    }
    else if (withExtAGT.existsAsFile())
    {
        RenIGXMeshLoader::instance().load(withExtAGT, pBuilder);
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
