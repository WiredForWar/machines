#pragma once

#include <map>
#include <memory>
#include "gui/painter.hpp"

//
// Ideally, clients should utilize the named bitmaps feature
//
class GuiRootSharedBitmaps final
{
public:
    GuiRootSharedBitmaps();
    explicit GuiRootSharedBitmaps(const IGuiPainter& painter);

    ~GuiRootSharedBitmaps();

    // Create or reassign a named bitmap, loading if necessary
    void createUpdateNamedBitmap(const char* name, const char* image);
    void createUpdateNamedBitmap(const std::string& name, const std::string& image);

    // Load a bitmap unless its already been loaded
    //  This method will be invoked by createUpdateNamedBitmap
    void loadSharedBitmap(const char* image);
    void loadSharedBitmap(const std::string& image);

    // Retrieve an OWNING REFERENCE to a named bitmap, e.g.: "backdrop"
    std::shared_ptr<GuiBitmap> getNamedBitmap(const char* name) const noexcept;
    std::shared_ptr<GuiBitmap> getNamedBitmap(const std::string& name) const noexcept;

    // Retrieve a NON-OWNING REFERENCE to a shared bitmap, associated by filename/path
    std::weak_ptr<GuiBitmap> getSharedBitmap(const char* image) const noexcept;
    std::weak_ptr<GuiBitmap> getSharedBitmap(const std::string& image) const noexcept;

private:
    const IGuiPainter& guiPainter_;

    std::map<std::string, std::shared_ptr<GuiBitmap>> sharedBitmaps_;
    std::map<std::string, std::string>                namedBitmaps_;
};
