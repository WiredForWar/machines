#include "FontMetrics.hpp"

#include "render/TextOptions.hpp"
#include "render/internal/FontImpl.hpp"

namespace Render
{

FontMetrics::FontMetrics(const Font& font)
    : font_(font)
{
}

int FontMetrics::horizontalAdvance(const std::string& text, const TextOptions& options) const
{
    const Render::FontImpl& font = *Render::FontImpl::get(&font_);
    const Render::FontImpl::CharData* charData {};

    int textWidth = 0;
    int lineTextWidth = 0;

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        uint character = text[i];
        if (character == '\n')
        {
            textWidth = std::max(textWidth, lineTextWidth);
            lineTextWidth = 0;
            continue;
        }

        charData = font.getChar(character);
        // Ignore missing characters
        if (!charData)
            continue;

        /* Advance the cursor to the start of the next character */
        lineTextWidth += charData->ax;
    }
    textWidth = std::max(textWidth, lineTextWidth);
    if (options.hasShadow())
    {
        textWidth += options.shadowX();
    }

    return textWidth;
}

int FontMetrics::horizontalAdvance(const std::string& text) const
{
    return horizontalAdvance(text, {});
}

} // namespace Render
