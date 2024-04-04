#include "gui/gui.hpp"
#include "system/pathname.hpp"
#include "gui/RootSharedBitmaps.hpp"

GuiRootSharedBitmaps::GuiRootSharedBitmaps()
    : GuiRootSharedBitmaps(GuiPainter::instance())
{
}

GuiRootSharedBitmaps::GuiRootSharedBitmaps(const IGuiPainter& painter)
    : guiPainter_(painter)
{
}

GuiRootSharedBitmaps::~GuiRootSharedBitmaps()
{
    namedBitmaps_.clear();
    sharedBitmaps_.clear();
}

void GuiRootSharedBitmaps::createUpdateNamedBitmap(
    const std::string& imageName,
    const std::string& filePath,
    float scale)
{
    loadSharedBitmap(filePath, scale);
    namedBitmaps_[imageName] = filePath;
}

void GuiRootSharedBitmaps::loadSharedBitmap(const std::string& filePath, float scale)
{
    auto sharedBitmap = sharedBitmaps_.find(filePath);
    // only load once
    if (sharedBitmap == sharedBitmaps_.end())
    {
        sharedBitmaps_[filePath] = std::make_shared<GuiBitmap>(Gui::requestScaledImage(filePath, scale));
    }
}

std::shared_ptr<GuiBitmap> GuiRootSharedBitmaps::getNamedBitmap(const std::string& imageName) const noexcept
{
    auto sharedBitmapsKey = std::string {};
    try
    {
        sharedBitmapsKey = namedBitmaps_.at(imageName);
    }
    catch (const std::out_of_range& ore)
    {
        std::cerr << "GuiRootSharedBitmaps::getNamedBitmap(): " << imageName << " was never created!!!" << std::endl;
        return std::shared_ptr<GuiBitmap>(nullptr);
    }

    return std::shared_ptr<GuiBitmap>(getSharedBitmap(sharedBitmapsKey));
}

std::weak_ptr<GuiBitmap> GuiRootSharedBitmaps::getSharedBitmap(const std::string& filePath) const noexcept
{
    try
    {
        return std::weak_ptr<GuiBitmap>(sharedBitmaps_.at(filePath));
    }
    catch (const std::out_of_range& ore)
    {
        std::cerr << "GuiRootSharedBitmaps::getSharedBitmap(): " << filePath << " was never loaded!!!" << std::endl;
        return std::weak_ptr<GuiBitmap>();
    }
}

void GuiRootSharedBitmaps::blitNamedBitmapFromArea(
    const std::shared_ptr<GuiBitmap>& bitmap,
    const Gui::Box& sourceArea,
    const Gui::Coord& destination,
    std::function<Gui::Box(const Gui::Box&)> fnSourceAreaTransform) const
{
    if (bitmap)
    {
        guiPainter_.blit(*bitmap, fnSourceAreaTransform(sourceArea), destination);
    }
    else
    {
        std::cerr << "GuiRootSharedBitmaps::blitNamedBitmapFromArea(): Attempted blit of null named bitmap."
                  << std::endl;
    }
}

void GuiRootSharedBitmaps::blitNamedBitmap(const std::shared_ptr<GuiBitmap>& bitmap, const Gui::Coord& destination)
    const
{
    if (bitmap)
    {
        guiPainter_.blit(*bitmap, destination);
    }
    else
    {
        std::cerr << "GuiRootSharedBitmaps::blitNamedBitmap(): Attempted blit of null named bitmap." << std::endl;
    }
}

int GuiRootSharedBitmaps::getWidthOfNamedBitmap(const std::shared_ptr<GuiBitmap>& bitmap) const
{
    int width = 0;

    if (bitmap)
    {
        if(!bitmap->requestedSize().isNull())
        {
            width = bitmap->requestedSize().width;
        }
        else
        {
            width = static_cast<int>(bitmap->width());
        }
    }
    else
    {
        std::cerr << "GuiRootSharedBitmaps::getWidthOfNamedBitmap: Null bitmap. Returning 0" << std::endl;
    }

    return width;
}

int GuiRootSharedBitmaps::getHeightOfNamedBitmap(const std::shared_ptr<GuiBitmap>& bitmap) const
{
    int height = 0;

    if (bitmap)
    {
        if(!bitmap->requestedSize().isNull())
        {
            height = bitmap->requestedSize().height;
        }
        else
        {
            height = static_cast<int>(bitmap->height());
        }
    }
    else
    {
        std::cerr << "GuiRootSharedBitmaps::getHeightOfNamedBitmap: Null bitmap. Returning 0" << std::endl;
    }

    return height;
}
