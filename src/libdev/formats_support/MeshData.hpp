#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace RenI {

struct MeshVertex
{
    float px{};
    float py{};
    float pz{};
    float nx{};
    float ny{};
    float nz{};
    float tu{};
    float tv{};
};

// Render-order sorting mode for a material.
enum class SortMode
{
    None,
    InterMeshCoplanar,
    IntraMeshAlpha,
    AbsoluteAlphaNegative,
    AbsoluteAlphaPositive,
};

// Spin-To-Face axis for billboard polygons.
enum class SpinAxis
{
    None,
    XYZ,
    X,
    Y,
    Z,
};

// Game-specific material description.
struct MeshMaterial
{
    float diffuseR{};
    float diffuseG{};
    float diffuseB{};
    float diffuseA{1.0f};

    float emissiveR{};
    float emissiveG{};
    float emissiveB{};

    std::string textureName;

    SortMode sortMode{};
    short sortPriority{};

    // >0 means the polygon is self-lit; diffuse was stored in emissive.
    float emissiveFactor{};

    SpinAxis spinAxis{};
    bool backfaceCull{true};
};

// One draw group: a set of triangles sharing a single material.
struct MeshPrimitive
{
    // Vertices local to this primitive (indices are relative to this array).
    std::vector<MeshVertex> vertices;

    // Triangle indices (3 per triangle, into `vertices`).
    std::vector<uint32_t> indices;

    int materialIndex{-1};
};

// Complete intermediate representation of one mesh (one RenMesh).
struct MeshData
{
    std::string name;
    std::vector<MeshMaterial> materials;
    std::vector<MeshPrimitive> primitives;
};

// ---------------------------------------------------------------------------
// Hierarchy intermediate representation.
// ---------------------------------------------------------------------------

// A single node in the mesh hierarchy tree.
// The transform is stored as a 3x3 rotation matrix (column-major basis
// vectors) plus a translation, matching MexTransform3d's constructor.
struct HierarchyNode
{
    // Rotation basis vectors (columns of the 3x3 matrix).
    float xBasisX{1}, xBasisY{}, xBasisZ{};
    float yBasisX{}, yBasisY{1}, yBasisZ{};
    float zBasisX{}, zBasisY{}, zBasisZ{1};

    // Translation.
    float posX{}, posY{}, posZ{};

    // Instance/link name (always set).
    std::string instanceName;

    // Mesh name (empty if this node carries no mesh).
    std::string meshName;

    // Path to the source file (used by the renderer to locate the mesh data).
    std::string filePath;

    // Scale factor for the mesh (typically 1.0).
    double scale{1.0};

    // Child nodes.
    std::vector<HierarchyNode> children;
};

// Complete hierarchy loaded from a file.
struct HierarchyData
{
    std::vector<HierarchyNode> roots;
};

// ---------------------------------------------------------------------------
// Animation intermediate representation.
// ---------------------------------------------------------------------------

// A single keyframe for a link animation channel.
struct AnimationKeyframe
{
    float time{}; // seconds

    // Rotation as quaternion (x, y, z, w).
    float qx{};
    float qy{};
    float qz{};
    float qw{1.0f};

    // Translation.
    float tx{};
    float ty{};
    float tz{};
};

// Animation channel for one link/node.
struct AnimationChannel
{
    std::string linkName;
    std::vector<AnimationKeyframe> keyframes;
};

// A named animation (corresponds to one W4dCompositePlan).
struct AnimationSet
{
    std::string name;
    std::vector<AnimationChannel> channels;
};

// All animations loaded from a file.
struct AnimationData
{
    std::vector<AnimationSet> animations;
};

} // namespace RenI
