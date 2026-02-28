#pragma once

#include "formats_support/IMeshLoader.hpp"
#include "system/pathname.hpp"

#include <unordered_map>
#include <memory>
#include <string>

class GltfMeshLoader final : public IMeshLoader
{
public:
    GltfMeshLoader();
    ~GltfMeshLoader() override;

    std::vector<std::string> supportedExtensions() const override;
    RenI::MeshData loadMesh(const SysPathName& path, const std::string& meshName) override;
    RenI::HierarchyData loadHierarchy(const SysPathName& path) override;
    void deleteAll() override;

private:
    // Opaque per-file cache entry (holds tinygltf::Model).
    struct FileEntry;

    using FileMap = std::unordered_map<std::string, std::unique_ptr<FileEntry>>;

    FileMap files_;

    FileEntry* loadFile(const SysPathName& pathName);
};
