/*
 * R E S O U R C E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "afx/resource.hpp"

#include "utility/rapidxml_utils.hpp"

#include <string_view>

#include <cstddef>

namespace
{

// Rewrites UTF-8 text as one byte per character, for a renderer that still reads a string
// that way. Both font systems index a glyph by the byte, so a two-byte character reaches
// them as two and each half draws whatever sits at that index -- German comes out as
// "auswAxhlen". Converting here, where every string enters the table, keeps the file in the
// encoding an editor can handle while the drawing code catches up.
//
// Temporary, and meant to be deleted whole by whatever teaches the fonts to read
// characters.
//
// Text that is not well-formed UTF-8 comes back exactly as it went in, so a translation
// still in the old encoding keeps working and converting twice changes nothing. A character
// with no single-byte form becomes a question mark.
std::string utf8ToLatin1(std::string_view text)
{
    std::string converted;
    converted.reserve(text.size());

    for (std::size_t pos = 0; pos < text.size();)
    {
        const unsigned char lead = text[pos];
        if (lead < 0x80)
        {
            converted += static_cast<char>(lead);
            ++pos;
            continue;
        }

        std::size_t continuationBytes = 0;
        char32_t codepoint = 0;
        char32_t smallestEncodable = 0;
        if (lead >= 0xC2 && lead <= 0xDF)
        {
            continuationBytes = 1;
            codepoint = lead & 0x1F;
            smallestEncodable = 0x80;
        }
        else if (lead >= 0xE0 && lead <= 0xEF)
        {
            continuationBytes = 2;
            codepoint = lead & 0x0F;
            smallestEncodable = 0x800;
        }
        else if (lead >= 0xF0 && lead <= 0xF4)
        {
            continuationBytes = 3;
            codepoint = lead & 0x07;
            smallestEncodable = 0x10000;
        }
        else
        {
            return std::string(text);
        }

        if (pos + continuationBytes >= text.size())
            return std::string(text);

        for (std::size_t i = 1; i <= continuationBytes; ++i)
        {
            const unsigned char byte = text[pos + i];
            if ((byte & 0xC0) != 0x80)
                return std::string(text);

            codepoint = (codepoint << 6) | (byte & 0x3F);
        }

        // An overlong sequence spells what a shorter one already spells, and a surrogate
        // half spells nothing. Either way these bytes are not UTF-8, so one bad one says
        // the whole string is in the old encoding.
        if (codepoint < smallestEncodable || codepoint > 0x10FFFF
            || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
        {
            return std::string(text);
        }

        converted += codepoint <= 0xFF ? static_cast<char>(codepoint) : '?';
        pos += 1 + continuationBytes;
    }

    return converted;
}

} // namespace

bool AfxResourceLib::addStringsFromFile(const SysPathName& path)
{
    PRE(path.existsAsFile());

    bool parsed = true;
    rapidxml::file<> xmlFile(path.c_str()); // Default template is char
    rapidxml::xml_document<> doc;
    try
    {
        doc.parse<0>(xmlFile.data());
    }
    catch (const rapidxml::parse_error& e)
    {
        parsed = false;
        std::cerr << e.what() << " here: " << e.where<char>() << std::endl;
    }

    if (parsed)
    {
        rapidxml::xml_node<>* node = doc.first_node();

        for (rapidxml::xml_node<>* a = node->first_node(); a; a = a->next_sibling())
        {
            resourceStrings_[atoi(a->first_attribute()->value())] = utf8ToLatin1(a->value());
        }
    }

    return parsed;
}

std::string AfxResourceLib::getString(const uint id) const
{
    if (resourceStrings_.count(id) > 0)
        return resourceStrings_.at(id);
    else
        return "";
}
