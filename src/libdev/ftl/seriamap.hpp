/*
 * S E R I A M A P . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    FtlSerialMap

    Implements a map from FtlSerialId to objects of class VALUE, over
    which the class is parameterized.
*/

#ifndef _FTL_SERIAMAP_HPP
#define _FTL_SERIAMAP_HPP

#include "base/base.hpp"

#include "ftl/map.hpp"
#include "ftl/serialid.hpp"
#include "ctl/vector.hpp"

// Orthodox canonical
template <class VALUE> class FtlSerialMap : public FtlMap<FtlSerialId, VALUE>
{
public:
    // ctor.
    FtlSerialMap();

    // dtor.
    ~FtlSerialMap() override {};

    // inherited from FtlContainer...
    bool contains(const FtlSerialId& id) const override;

    // inherited from FtlAssociativeCollection...
    void add(const FtlPair<FtlSerialId, VALUE>& addMe) override;
    // PRE( not contains( addMe.first )

    void remove(const FtlSerialId& id) override;
    // PRE( contains( id ) )

    void CLASS_INVARIANT;

protected:
    const VALUE& doSubscript(const FtlSerialId& id) const override;
    // PRE( contains( id ) );

private:
    // Operations deliberately revoked
    FtlSerialMap(const FtlSerialMap<VALUE>&);
    FtlSerialMap<VALUE>& operator=(const FtlSerialMap<VALUE>&);
    bool operator==(const FtlSerialMap<VALUE>&);

    using Entry = std::pair<size_t, VALUE>;

    // Data members
    ctl_vector<size_t> index_; // index_[id] is the element of values_ storing the mapped value
    ctl_vector<Entry> values_; // first of pair is location in index_ of ref
};

// #ifdef _INSTANTIATE_TEMPLATE_CLASSES
#include "ftl/seriamap.ctp"
// #endif

#ifdef _INLINE
#include "ftl/seriamap.itp"
#endif

#endif

/* End SERIAMAP.HPP *************************************************/
