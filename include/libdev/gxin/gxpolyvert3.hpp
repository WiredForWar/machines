/*
 * G X P O L Y V E R T 3 . H P P 
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    GXPolyVert3

    A brief description of the class should go in here
*/

#ifndef _GXPOLYVERT3_HPP
#define _GXPOLYVERT3_HPP

#include "base/base.hpp"
#include "ctl/vector.hpp"
#include "gxin/gxcolor.hpp"
#include "gxin/gxuvcoords.hpp"

class GXPolyVert3
// Canonical form revoked
{
public:
    GXPolyVert3( void );
    ~GXPolyVert3( void );
    GXPolyVert3( const GXPolyVert3& );
    GXPolyVert3& operator =( const GXPolyVert3& );
    friend bool operator ==( const GXPolyVert3&, const GXPolyVert3& );
    friend bool operator < ( const GXPolyVert3&, const GXPolyVert3& );

    void CLASS_INVARIANT;

    friend ostream& operator <<( ostream& o, const GXPolyVert3& t );

	USHORT pointIndex( void ) const { return pointIndex_; }
	void pointIndex(USHORT newPointIndex) { pointIndex_=newPointIndex; }
	USHORT normalIndex( void ) const { return normalIndex_;	}
	void normalIndex(USHORT newNormalIndex) { normalIndex_=newNormalIndex; }

	void hasColor(bool newHasColor);
	bool hasColor( void ) const { return ! color_==NULL; }

	const GXColor& color( void ) const {
	  PRE(hasColor());
	  return *color_;
	}

	void color(const GXColor& newColor) {
	  hasColor(true);
	  *color_=newColor;
	}

	const GXUVCoords& uv( void ) const { return uv_; }
	void uv(const GXUVCoords& newUV) { uv_=newUV; }

private:
	USHORT pointIndex_;
	USHORT normalIndex_;
	GXColor *color_;
    GXUVCoords uv_;
};

#endif

/* End GXPOLYVERT3.HPP **********************************************/
