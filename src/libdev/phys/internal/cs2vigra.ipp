/*
 * C S 2 V I G R A . I P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of inline non-template methods and inline global functions

#ifdef _INLINE
#define _CODE_INLINE inline
#else
#define _CODE_INLINE
#endif
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
bool PhysCS2dVisibilityGraph::isUpToDate() const
{
    return upToDate_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
bool PhysCS2dVisibilityGraph::isFindPathFinished() const
{
    PRE(findPathIsDefined_);
    return upToDate_ && findPathCompleted_;
}
//////////////////////////////////////////////////////////////////////////////////////////
/* End CS2VIGRA.IPP *************************************************/
