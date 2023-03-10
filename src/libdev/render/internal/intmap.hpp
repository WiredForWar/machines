/*
 * I N T M A P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#ifndef _RENDER_INTMAP_HPP
#define _RENDER_INTMAP_HPP

#include "base/base.hpp"
#include "render/render.hpp"
#include "mathex/mathex.hpp"
#include "ctl/vector.hpp"

class RenIVtxIntensity
{
public:
    Ren::VertexIdx index() const { return index_; }

    double intensity() const { return int_; }

    // For the purposes of running find on the intensity map, this is useful.

    bool operator==(const RenIVtxIntensity& r) const { return index_ == r.index_; }

private:
    friend class RenIVertexIntensities;

    Ren::VertexIdx index_;
    double int_;
};

// Provide a mapping from meshed vertex index to vertex intensity.
// We require traversal which is as efficient as possible, but we don't
// particularly require efficient search, so an STL map is *not* used.

class RenIVertexIntensities : private ctl_vector<RenIVtxIntensity>
{
public:
    using Base = ctl_vector<RenIVtxIntensity>;

    RenIVertexIntensities(size_t nElements);
    RenIVertexIntensities(const RenIVertexIntensities& copyMe);
    ~RenIVertexIntensities() override;

    // Does this map contain an entry for the given index?
    // POST(implies(result == true  && value, *value!=1));
    // POST(implies(result == false && value, *value==1));
    bool contains(Ren::VertexIdx i, double* value = nullptr) const;

    // POST(implies(!contains(i), result==1));
    double value(Ren::VertexIdx i) const;

    // POST(implies(newValue==1, !contains(i)));
    // POST(implies(newValue!=1,  contains(i)));
    void value(Ren::VertexIdx i, double newValue);

    using ctl_vector<RenIVtxIntensity>::begin;
    using ctl_vector<RenIVtxIntensity>::end;

private:
    // Operation deliberately revoked
    // RenIVertexIntensities( const RenIVertexIntensities& );
    RenIVertexIntensities& operator=(const RenIVertexIntensities&);
    bool operator==(const RenIVertexIntensities&);
};

// This is a less-sparse exapansion of a RenIVertexIntensities.

class RenIExpandedIntensityMap : public ctl_vector<double>
{
public:
    using Base = ctl_vector<double>;

    RenIExpandedIntensityMap(size_t nElements); // POST(size() >= nElements);
    void checkSize(size_t nElements); // POST(size() >= nElements);
    void expand(const RenIVertexIntensities*);
};

#endif

/* End INTMAP.HPP ***************************************************/
