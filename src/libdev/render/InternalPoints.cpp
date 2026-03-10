/*
 * I N P O I N T S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "render/internal/InternalPoints.hpp"
#include "render/internal/VertexData.hpp"

RenIPoints::RenIPoints(size_t initial)
    : ctl_min_memory_vector<RenIVertex>(initial)
{
}

/* End INPOINTS.CPP *************************************************/
