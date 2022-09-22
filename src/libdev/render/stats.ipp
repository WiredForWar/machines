/*
 *  S T A T S . I P P 
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifdef _INLINE
    #define _CODE_INLINE    inline
#else
    #define _CODE_INLINE
#endif

_CODE_INLINE
void RenStats::incrPolyCount(uint32_t number)
{
	polygonsDrawn_ += number;
}

_CODE_INLINE
void RenStats::incrLineCount(uint32_t number)
{
	linesDrawn_ += number;
}

_CODE_INLINE
void RenStats::incrTTFCount(uint32_t number)
{
	TTFsDrawn_ += number;
}

_CODE_INLINE
void RenStats::incrSTFCount(uint32_t number)
{
	STFsDrawn_ += number;
}

_CODE_INLINE
void RenStats::incrPointsCount(uint32_t number)
{
	pointsDrawn_ += number;
}

_CODE_INLINE
uint32_t RenStats::frameCount() const
{
	return frameCount_;
}	

/* End STATS.IPP **************************************************/
