/*
 * A S T A R V T X . C T P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline template methods

#ifndef _INLINE
#include "graph/astarvtx_inline_templates.hpp"
#endif

//////////////////////////////////////////////////////////////////////////////////////////

template <class GRA_GRAPH>
GraAStarVertex<GRA_GRAPH>::GraAStarVertex()
    : state_(OPEN)
{

    TEST_INVARIANT;
}
//////////////////////////////////////////////////////////////////////////////////////////

template <class GRA_GRAPH>
GraAStarVertex<GRA_GRAPH>::GraAStarVertex(
    const Weight& costFromStart,
    const Weight& estimatedCostToEnd,
    const ArcId& previousArcId)
    : estimatedCostToEnd_(estimatedCostToEnd)
    , costFromStart_(costFromStart)
    , previousArcId_(previousArcId)
    , state_(OPEN)
{

    TEST_INVARIANT;
}
//////////////////////////////////////////////////////////////////////////////////////////

template <class GRA_GRAPH> GraAStarVertex<GRA_GRAPH>::~GraAStarVertex()
{
    TEST_INVARIANT;
}
//////////////////////////////////////////////////////////////////////////////////////////

template <class GRA_GRAPH> void GraAStarVertex<GRA_GRAPH>::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}
//////////////////////////////////////////////////////////////////////////////////////////

template <class GRA_GRAPH> std::ostream& operator<<(std::ostream& o, const GraAStarVertex<GRA_GRAPH>& t)
{

    o << "GraAStarVertex " << static_cast<const void*>(&t) << " cost " << t.costFromStart_ << " estimate " << t.estimatedCostToEnd_
      << " arc " << t.previousArcId_ << (t.closed() ? " CLOSED" : " OPEN ") << std::endl;

    return o;
}
//////////////////////////////////////////////////////////////////////////////////////////
/* End ASTARVTX.CTP *************************************************/
