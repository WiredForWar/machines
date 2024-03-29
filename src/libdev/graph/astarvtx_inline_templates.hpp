/*
 * A S T A R V T X . I T P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of inline template methods

#ifdef _INLINE
#define _CODE_INLINE inline
#else
#define _CODE_INLINE
#endif

//////////////////////////////////////////////////////////////////////////////////////////
template <class GRA_GRAPH>
_CODE_INLINE const typename GraAStarVertex<GRA_GRAPH>::Weight& GraAStarVertex<GRA_GRAPH>::costFromStart() const
{
    return costFromStart_;
}
//////////////////////////////////////////////////////////////////////////////////////////
template <class GRA_GRAPH> _CODE_INLINE void GraAStarVertex<GRA_GRAPH>::costFromStart(const Weight& cost)
{
    costFromStart_ = cost;
}
//////////////////////////////////////////////////////////////////////////////////////////
template <class GRA_GRAPH>
_CODE_INLINE const typename GraAStarVertex<GRA_GRAPH>::Weight& GraAStarVertex<GRA_GRAPH>::estimatedCostToEnd() const
{
    return estimatedCostToEnd_;
}
//////////////////////////////////////////////////////////////////////////////////////////
template <class GRA_GRAPH>
_CODE_INLINE const typename GraAStarVertex<GRA_GRAPH>::ArcId& GraAStarVertex<GRA_GRAPH>::previousArcId() const
{
    return previousArcId_;
}
//////////////////////////////////////////////////////////////////////////////////////////
template <class GRA_GRAPH> _CODE_INLINE void GraAStarVertex<GRA_GRAPH>::previousArcId(const ArcId& arcId)
{
    previousArcId_ = arcId;
}
//////////////////////////////////////////////////////////////////////////////////////////
template <class GRA_GRAPH> _CODE_INLINE bool GraAStarVertex<GRA_GRAPH>::closed() const
{
    return state_ == CLOSED;
}
//////////////////////////////////////////////////////////////////////////////////////////
template <class GRA_GRAPH> _CODE_INLINE void GraAStarVertex<GRA_GRAPH>::closed(bool isClosed)
{
    state_ = (isClosed ? CLOSED : OPEN);
}
//////////////////////////////////////////////////////////////////////////////////////////
/* End ASTARVTX.ITP *************************************************/
