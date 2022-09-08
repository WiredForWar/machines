/*
 * M A T H E X . C P P
 * (c) Charybdis Limited, 1995, 1996. All Rights Reserved.
 */

//////////////////////////////////////////////////////////////////////

#include <limits.h>

#include "mathex/mathex.hpp"

#ifndef _INLINE
    #include "mathex/mathex.ipp"
#endif

const double Mathex::PI         = 3.14159265358979323846;
const double Mathex::PI_DIV_2   = PI / 2.0;
const double Mathex::SQRT_2     = sqrt( 2.0 );

//////////////////////////////////////////////////////////////////////
//static
Mathex::Side Mathex::side( MATHEX_SCALAR h )
{
	MATHEX_SCALAR epsilon = MexEpsilon::instance();
	Side result;

	if( h > epsilon ) result = VANILLA;
	else if( h < -epsilon ) result = CHOCOLATE;
	else result = ONEDGE;

	return result;
}
//////////////////////////////////////////////////////////////////////
/* End MATHEX.CPP *****************************************************/
