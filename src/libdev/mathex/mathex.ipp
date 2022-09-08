/*
 * M A T H E X . I P P
 * (c) Charybdis Limited, 1995. All Rights Reserved.
 */

#include <stdlib.h>

#ifdef  _INLINE
    #define _CODE_INLINE    inline
#else
    #define _CODE_INLINE
#endif

//////////////////////////////////////////////////////////////////////////

 _CODE_INLINE
int Mathex::abs( int x )
{
	return  x < 0 ? -x : x;
}

_CODE_INLINE
long Mathex::abs( long x )
{
	return labs( x );
}

_CODE_INLINE
float Mathex::abs( float x )
{
	return fabs( x );
}

_CODE_INLINE
double Mathex::abs( double x )
{
	return fabs( x );
}

_CODE_INLINE
long double
Mathex::abs( long double x )
{
	return  fabs( (double)x );
}

//////////////////////////////////////////////////////////////////////

_CODE_INLINE float mod( float a, float b ) { return fmod( a, b ); }
_CODE_INLINE double mod( double a, double b ) { return fmod( a, b ); }
_CODE_INLINE long double mod( long double a, long double b ) { return fmod( (double)a, (double)b ); }

//////////////////////////////////////////////////////////////////////
_CODE_INLINE
//static
Mathex::Side Mathex::side( Side a, Side b )
{
	return ( a == b ? a : STRADDLE );
}
//////////////////////////////////////////////////////////////////////
/* End MATHEX.IPP *****************************************************/
