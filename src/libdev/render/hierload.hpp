/*
 * H I E R L O A D . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    RenHierarchyLoader

    A brief description of the class should go in here
*/

#ifndef _RENDER_HIERLOAD_HPP
#define _RENDER_HIERLOAD_HPP

#include "base/base.hpp"

class SysPathName;
class RenHierarchyBuilder;

namespace RenI { struct HierarchyNode; }

class RenHierarchyLoader
{
public:
    static void load(const SysPathName& pathName, RenHierarchyBuilder* pBuilder);

    // Convert a HierarchyNode tree into RenHierarchyBuilder::addFrame calls.
    static void buildFrame(const RenI::HierarchyNode& node, RenHierarchyBuilder* builder);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const RenHierarchyLoader& t);

private:
    // Operation deliberately revoked
    RenHierarchyLoader();

    // Operation deliberately revoked
    ~RenHierarchyLoader();

    // Operation deliberately revoked
    RenHierarchyLoader(const RenHierarchyLoader&);

    // Operation deliberately revoked
    RenHierarchyLoader& operator=(const RenHierarchyLoader&);

    // Operation deliberately revoked
    bool operator==(const RenHierarchyLoader&);
};

#endif

/* End HIERLOAD.HPP *************************************************/
