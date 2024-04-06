#pragma once

#include <string>

namespace Render
{

class Font;
class TextOptions;

class FontMetrics
// Canonical form revoked
{
public:
    FontMetrics(const Font& font);

    int horizontalAdvance(const std::string& text, const TextOptions &options) const;
    int horizontalAdvance(const std::string& text) const;

private:
    const Font &font_;
};

} // Render namespace
