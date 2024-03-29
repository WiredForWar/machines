#ifndef INCLUDED_XFILETYPES_H
#define INCLUDED_XFILETYPES_H

#define MAX_NUMBER_OF_TEXTURECOORDS 3
#define MAX_NUMBER_OF_COLOR_SETS 3
namespace XFile {
using Matrix4x4 = union Matrix4x4
{
    float data[4][4];
    struct
    {
        float a1, a2, a3, a4;
        float b1, b2, b3, b4;
        float c1, c2, c3, c4;
        float d1, d2, d3, d4;
    };
    Matrix4x4()
        : a1(1)
        , a2(0)
        , a3(0)
        , a4(0)
        , b1(0)
        , b2(1)
        , b3(0)
        , b4(0)
        , c1(0)
        , c2(0)
        , c3(1)
        , c4(0)
        , d1(0)
        , d2(0)
        , d3(0)
        , d4(1) {};
};

using Vector2D = struct Vector2D
{
    float x;
    float y;
};
using Vector3D = struct Vector3D
{
    float x;
    float y;
    float z;
};
using Color3D = struct Color3D
{
    float r;
    float g;
    float b;
};
using Color4D = struct Color4D
{
    float r;
    float g;
    float b;
    float a;
    Color4D() { }
    Color4D(float r1, float g1, float b1, float a1)
        : r(r1)
        , g(g1)
        , b(b1)
        , a(a1)
    {
    }
};

using VectorKey = struct VectorKey
{
    double mTime;
    Vector3D mValue;
};
using QuatKey = struct QuatKey
{
    double mTime;
    struct
    {
        float w, x, y, z;
    } mValue;
};

class DeadlyImportError : public std::exception
{
public:
    DeadlyImportError(const char* msg)
        : msg_(msg)
    {
    }
    DeadlyImportError(const std::string msg)
        : msg_(msg)
    {
    }
    ~DeadlyImportError() throw() override { }
    const char* what() const throw() override { return std::string("Deadly Import Error: " + msg_).c_str(); }

private:
    std::string msg_;
};

} // namespace XFile

#endif
