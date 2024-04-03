/*
 * R E N D E R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_RENDER_HPP
#define _RENDER_RENDER_HPP

#include "base/base.hpp"

class RenMesh;
class RenMaterialVec;
class RenUVTransform;
template <class T> class CtlCountedPtr;
template <class T> class CtlConstCountedPtr;

// Various widely used typedefs.  These could correctly be put in class
// scopes, e.g. RenTexManager::TexId.  However, putting them here helps
// reduce dependencies.
// Hence, this file is supposed to be **small & simple**.
namespace Ren
{
    static const uint NullTexId = 0;
    // This returns RenDevice::current()->out for classes which don't need the
    // full definition of RenDevice.
    // PRE(RenDevice::current());
    std::ostream& out();

    // This invokes the various render library Singletons in an order which
    // will guarantee correct destruction order.  This must be called before
    // any other render library Singletons are invoked.
    // Chose a co-ordinate system before starting the render lib.
    // PRE(MexCoordSystem::instance().isSet());
    void initialise();
    bool initialised(); // a pre-condition for many things

    using VertexIdx = ushort;
    using TriangleIdx = ushort;
    using MaterialIdx = uchar;
    using TexId = uint;
    // TODO init
    // static const uint NullTexId;

    // These are maximum values per mesh.
    enum
    {
        maxVertices = 65535,
        maxTriangles = 65535,
        maxMaterials = 255
    };

    using MeshPtr = CtlCountedPtr<RenMesh>;
    using ConstMeshPtr = CtlConstCountedPtr<RenMesh>;

    using MaterialVecPtr = CtlCountedPtr<RenMaterialVec>;
    using ConstMaterialVecPtr = CtlConstCountedPtr<RenMaterialVec>;

    using UVTransformPtr = CtlCountedPtr<RenUVTransform>;
    using ConstUVTransformPtr = CtlConstCountedPtr<RenUVTransform>;

    struct Size
    {
        constexpr Size() = default;
        constexpr Size(int w, int h)
            : width(w)
            , height(h)
        {
        }

        template<typename T>
        constexpr Size& operator*=(T multiplier)
        {
            width *= multiplier;
            height *= multiplier;
            return *this;
        }

        constexpr bool isNull() const { return width == 0 && height == 0; }

        int width {};
        int height {};
    };

    struct Rect
    {
        constexpr Rect(int x = 0, int y = 0, int w = 0, int h = 0)
            : originX(x)
            , originY(y)
            , width(w)
            , height(h)
        {
        }
        int originX, originY, width, height;
    };


    template<typename T>
    constexpr inline Size operator*(Size rhs, T c)
    {
        return rhs *= c;
    }

    enum LightType
    {
        DIRECTIONAL,
        POINT,
        UNIFORM
    };
    };

    std::ostream& operator<<(std::ostream&, const Ren::Rect&);

#endif

/* End RENDER.HPP ***************************************************/
