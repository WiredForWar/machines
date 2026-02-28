#pragma once

#include "formats_support/MeshData.hpp"

#include <string>
#include <vector>

class SysPathName;

class IMeshLoader
{
public:
    virtual ~IMeshLoader() = default;

    // File extensions this loader can handle (lowercase, no dot).
    // First element is the preferred/primary extension.
    virtual std::vector<std::string> supportedExtensions() const = 0;

    // Load a named mesh from a file. Returns an empty MeshData (no primitives) on failure.
    virtual RenI::MeshData loadMesh(const SysPathName& path, const std::string& meshName) = 0;

    // Load the scene hierarchy from a file. Returns an empty HierarchyData (no roots) on failure.
    virtual RenI::HierarchyData loadHierarchy(const SysPathName& path) = 0;

    // Release all cached data.
    virtual void deleteAll() = 0;
};
