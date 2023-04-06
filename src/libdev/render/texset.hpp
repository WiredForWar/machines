/*
 * T E X S E T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    RenTextureSet

    A brief description of the class should go in here
*/

#ifndef _TEXSET_HPP
#define _TEXSET_HPP

#include "base/base.hpp"
#include "base/IProgressReporter.hpp"

template <class T> class ctl_vector;
class RenTexture;
class RenTextureSetImpl;
class SysPathName;

class RenTextureSet
// Canonical form revoked
{
public:
    RenTextureSet();
    // POST( not isLoaded() );
    RenTextureSet(const SysPathName& sysPathName);
    // POST( isLoaded() );
    RenTextureSet(const SysPathName& sysPathName, IProgressReporter* pReporter);
    // PRE( pReporter );
    // POST( isLoaded() );
    ~RenTextureSet();

    void CLASS_INVARIANT;

    // This function can only be called if the texture set has
    // not been loaded (i.e. the instance has been created by the default
    // constructor)
    void load(const SysPathName&);
    // PRE(not isLoaded() );
    void load(const SysPathName&, IProgressReporter* pReporter);
    // PRE(not isLoaded() );
    // PRE( pReporter );

    bool isLoaded() const;

    using RenTextures = ctl_vector<RenTexture>;

private:
    friend std::ostream& operator<<(std::ostream& o, const RenTextureSet& t);

    RenTextureSet(const RenTextureSet&);
    RenTextureSet& operator=(const RenTextureSet&);

    RenTextureSetImpl* pImpl_;
};

#endif

/* End TEXSET.HPP ***************************************************/
