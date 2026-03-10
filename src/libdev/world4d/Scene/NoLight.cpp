/*
 * N O L I G H T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "world4d/Scene/Internal/NoLight.hpp"
#include "world4d/Entity/Entity.hpp"
#include "render/MaterialMap.hpp"
#include "render/Material.hpp"

bool w4dNeedsLighting(const W4dEntity* entity)
{
    // Build a list of all the materials in the entity.
    RenMaterialSet matSet;
    entity->addMaterials(&matSet);

    // Iterate thru those materials looking for ones which need lighting.
    bool needsLighting = false;
    RenMaterialSet::const_iterator it = matSet.begin();
    while (it != matSet.end() && !needsLighting)
    {
        const RenMaterial& mat = *it;
        if (mat.needsLighting())
            needsLighting = true;

        ++it;
    }

    return needsLighting;
}

/* End NOLIGHT.CPP **************************************************/
