/*
 * M A T A P P L Y . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "render/internal/matapply.hpp"
#include "render/internal/lightbuf.hpp"
#include "render/internal/intmap.hpp"
#include "render/internal/colpack.hpp"
#include "render/internal/vtxdata.hpp"
#include "ctl/mmvector.hpp"

// The colour filter is multiplied into the material properties in order to
// avoid a per-vertex multiplication.

RenIMaterialApplicator::RenIMaterialApplicator( const RenIMatBody* body,
                                                const RenColour& ambient,
                                                const RenIExpandedIntensityMap* map,
                                                bool /*mapUnity*/, // ignored: per-vertex intensity disabled
                                                const RenColour& filter )
    : difR_( body->diffuse().r() * filter.r() )
    , difG_( body->diffuse().g() * filter.g() )
    , difB_( body->diffuse().b() * filter.b() )
    , constR_( filter.r() * ( ambient.r() * body->ambient().r() + body->emissive().r() ) )
    , constG_( filter.g() * ( ambient.g() * body->ambient().g() + body->emissive().g() ) )
    , constB_( filter.b() * ( ambient.b() * body->ambient().b() + body->emissive().b() ) )
    , alpha_( body->diffuse().packedAlpha() )
    , packedAmbient_( packColourChecked( constR_, constG_, constB_, alpha_ ) )
    , packedBlack_( packColourChecked( 0, 0, 0, 0 ) )
    , map_( map )
    , mapUnity_( true )
    , // per-vertex intensity disabled
    perVertexMats_( false )
    , specularRequired_( false )
    , filter_( filter )
{
}

void RenIMaterialApplicator::applySpecular( RenIVertex& vtx, float lambR, float lambG, float lambB ) const
{
    if ( specularRequired_ )
    {
        // Set the specular to a fraction of the light colour, *not* the
        // polygon colour.  What is the correct way to combine the filter
        // and overflow effects?  I think this will look OK.
        const float specR = ( lambR > 1 ) ? filter_.r() * ( lambR - 1 ) : 0;
        const float specG = ( lambG > 1 ) ? filter_.g() * ( lambG - 1 ) : 0;
        const float specB = ( lambB > 1 ) ? filter_.b() * ( lambB - 1 ) : 0;

        // Specular alpha is used for per-vertex fogging.  This is currently
        // unsupported, so the correct value is zero.
        vtx.specular = packColourChecked( specR, specG, specB, 0 );

        // RENDER_STREAM("packing: lamb=(" << lambR << ","  << lambG << ","  << lambB << ")");
        // RENDER_STREAM(" spec=(" << specR << ","  << specG << ","  << specB << ")\n");
    }
    else
    {
        vtx.specular = packedBlack_;
    }
}

RenILitMatApp::RenILitMatApp( const RenIMatBody* body,
                              const RenColour& ambient,
                              const RenIExpandedIntensityMap* map,
                              bool mapUnity,
                              const RenColour& filter )
    : RenIMaterialApplicator( body, ambient, map, mapUnity, filter )
{
}

RenINoOverflowMatApp::RenINoOverflowMatApp( const RenIMatBody* body,
                                            const RenColour& ambient,
                                            const RenIExpandedIntensityMap* map,
                                            bool mapUnity,
                                            const RenColour& filter )
    : RenILitMatApp( body, ambient, map, mapUnity, filter )
{
}

void RenINoOverflowMatApp::applyToVertex( RenIVertex& vtx, float lambR, float lambG, float lambB ) const
{
    const float r = lambR * difR_ + constR_;
    const float g = lambG * difG_ + constG_;
    const float b = lambB * difB_ + constB_;
    vtx.color = packColourUnChecked( r, g, b, alpha_ );
    applySpecular( vtx, lambR, lambG, lambB );
}

RenICheckedMatApp::RenICheckedMatApp( const RenIMatBody* body,
                                      const RenColour& ambient,
                                      const RenIExpandedIntensityMap* map,
                                      bool mapUnity,
                                      const RenColour& filter )
    : RenILitMatApp( body, ambient, map, mapUnity, filter )
{
}

void RenICheckedMatApp::applyToVertex( RenIVertex& vtx, float lambR, float lambG, float lambB ) const
{
    const float r = lambR * difR_ + constR_;
    const float g = lambG * difG_ + constG_;
    const float b = lambB * difB_ + constB_;
    vtx.color = packColourChecked( r, g, b, alpha_ );
    applySpecular( vtx, lambR, lambG, lambB );
}

RenIUnlitMatApp::RenIUnlitMatApp( const RenIMatBody* body,
                                  const RenColour& ambient,
                                  const RenIExpandedIntensityMap* map,
                                  bool mapUnity,
                                  const RenColour& filter )
    : RenIMaterialApplicator( body, ambient, map, mapUnity, filter )
{
}

void RenIUnlitMatApp::applyToVertex( RenIVertex& vtx ) const
{
    vtx.color = packedAmbient_;
    vtx.specular = packedBlack_;
}

void RenIUnlitMatApp::applyViaIndices
(
	const ctl_min_memory_vector<Ren::VertexIdx>& indices,
	RenILightingBuffer*	liteBuf
) const
{
	RenIVertex*  vertices   = liteBuf->vertices();
	bool*        pApplied   = liteBuf->matAppliedFlags();
	bool*		 pPerVertex = liteBuf->perVertexMatFlags();

	ctl_min_memory_vector<Ren::VertexIdx>::const_iterator it = indices.begin();
	while (it != indices.end())
	{
		if (!perVertexMats_ || !pPerVertex[*it])
		{
			bool& applied = pApplied[*it];
			if (!applied)
			{
				applied = true;
				RenIVertex& vtx = vertices[*it];
				applyToVertex(vtx);
			}
		}
		++it;
	}
}

// Defines a function which iterates over a vector of vertices and applies the
// lighting calculation to each vertex.
void RenIUnlitMatApp::applyDirectToVertices
(
	RenILightingBuffer*		liteBuf,
	size_t					nVertices
) const
{
	RenIVertex*  vertices   = liteBuf->vertices();
	bool*        pApplied   = liteBuf->matAppliedFlags();
	bool*		 pPerVertex = liteBuf->perVertexMatFlags();

	for (size_t i=nVertices; i!=0; --i)
	{
		if (!perVertexMats_ || !(*pPerVertex))
		{
			bool& applied = *pApplied;
			if (!applied)
			{
				applied = true;
				applyToVertex(*vertices);
			}
		}
		++vertices;
		++pPerVertex;
		++pApplied;
	}
}

// Defines a function which iterates over a vector of indices and applies the
// lighting calculation to every vertex referred to by the indices.
void RenILitMatApp::applyViaIndices( const ctl_min_memory_vector< Ren::VertexIdx >& indices,
                                     RenIFloatLightingBuffer* liteBuf ) const
{
    RenIVertex* vertices = liteBuf->vertices();
    const float* pR = liteBuf->rLamberts();
    const float* pG = liteBuf->gLamberts();
    const float* pB = liteBuf->bLamberts();
    bool* pApplied = liteBuf->matAppliedFlags();
    bool* pUnderflow = liteBuf->underflowFlags();
    bool* pPerVertex = liteBuf->perVertexMatFlags();

    ctl_min_memory_vector< Ren::VertexIdx >::const_iterator it = indices.begin();
    while ( it != indices.end() )
    {
        if ( !perVertexMats_ || !pPerVertex[ *it ] )
        {
            bool& applied = pApplied[ *it ];
            if ( !applied )
            {
                applied = true;

                if ( pUnderflow[ *it ] )
                    vertices[ *it ].color = packedAmbient_;
                else
                    applyToVertex( vertices[ *it ], pR[ *it ], pG[ *it ], pB[ *it ] );
            }
        }
        ++it;
    }
}

// Defines a function which iterates over a vector of vertices and applies the
// lighting calculation to each vertex.
void RenILitMatApp::applyDirectToVertices( RenIFloatLightingBuffer* liteBuf, size_t nVertices ) const
{
    RenIVertex* vertices = liteBuf->vertices();
    const float* pR = liteBuf->rLamberts();
    const float* pG = liteBuf->gLamberts();
    const float* pB = liteBuf->bLamberts();
    bool* pApplied = liteBuf->matAppliedFlags();
    bool* pUnderflow = liteBuf->underflowFlags();
    bool* pPerVertex = liteBuf->perVertexMatFlags();

    for ( size_t i = nVertices; i != 0; --i )
    {
        if ( !perVertexMats_ || !( *pPerVertex ) )
        {
            bool& applied = *pApplied;
            if ( !applied )
            {
                applied = true;

                if ( *pUnderflow )
                    vertices->color = packedAmbient_;
                else
                    applyToVertex( *vertices, *pR, *pG, *pB );
            }
        }
        ++vertices;
        ++pR;
        ++pG;
        ++pB;
        ++pPerVertex;
        ++pApplied;
        ++pUnderflow;
    }
}

/* End MATAPPLY.CPP *************************************************/
