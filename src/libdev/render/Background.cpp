/*
 * B A C K G R N D . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "render/Device.hpp"
#include "render/Background.hpp"

RenBackground::RenBackground()
    : vertices_(_NEW_ARRAY(RenIVertex, 4))
{
    vertices_[0].x = (-1);
    vertices_[1].x = (1);
    vertices_[2].x = (1);
    vertices_[3].x = (-1);

    vertices_[0].y = (-1);
    vertices_[1].y = (-1);
    vertices_[2].y = (1);
    vertices_[3].y = (1);

    vertices_[0].z = (0.99);
    vertices_[1].z = (0.99);
    vertices_[2].z = (0.99);
    vertices_[3].z = (0.99);

    vertices_[0].w = (0.01);
    vertices_[1].w = (0.01);
    vertices_[2].w = (0.01);
    vertices_[3].w = (0.01);

    vertices_[0].color = packColourChecked(1, 0, 1, 1);
    vertices_[1].color = vertices_[0].color;
    vertices_[2].color = vertices_[0].color;
    vertices_[3].color = vertices_[0].color;

    vertices_[0].specular = packColourChecked(0, 0, 0);
    vertices_[1].specular = vertices_[0].specular;
    vertices_[2].specular = vertices_[0].specular;
    vertices_[3].specular = vertices_[0].specular;

    vertices_[0].tu = (0);
    vertices_[0].tv = (0);
    vertices_[1].tu = (0);
    vertices_[1].tv = (0);
    vertices_[2].tu = (0);
    vertices_[2].tv = (0);
    vertices_[3].tu = (0);
    vertices_[3].tv = (0);
}

RenBackground::~RenBackground()
{
    _DELETE_ARRAY(vertices_);
}

void RenBackground::render() const
{
}

/* End BACKGRND.CPP *************************************************/
