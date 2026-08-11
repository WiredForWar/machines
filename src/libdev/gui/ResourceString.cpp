/*
 * R E S T R I N G . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "gui/ResourceString.hpp"
#include "ctl/Vector.hpp"

#include <ctype.h>

#include "afx/resource.hpp"

//////////////////////////////////////////////////////////////////////

GuiResourceString::GuiResourceString(Gui::StringId id)
{
    PRE(hasResource());

    insertionString_ = Gui::resourceStringText(id);
}

//////////////////////////////////////////////////////////////////////

GuiString Gui::substituteArguments(std::string_view text, const GuiStrings& arguments)
// <ins str> ::= { <esc seq> | <non-percent-character> }
// <esc seq> ::= <number esc seq> | <percent esc seq>
// <percent esc seq> ::= <%> <%>
// <number esc seq> ::=  <%> <number> [ <%> ]
// <number>         ::= <leadingdigit> { <digit> }
// <leadingdigit>   ::= <1> - <9>
// <digit>          ::= <0> - <9>
{
    GuiString result;
    result.reserve(text.size());

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] != '%')
        {
            result += text[i];
            continue;
        }

        // A percent sign at the very end has nothing to introduce.
        if (i + 1 == text.size())
        {
            result += '%';
            break;
        }

        if (text[i + 1] == '%')
        {
            result += '%';
            ++i;
            continue;
        }

        std::size_t number = 0;
        std::size_t end = i + 1;
        while (end < text.size() && isdigit(static_cast<unsigned char>(text[end])))
        {
            // Once the number is too big to name an argument it can stay that way,
            // which keeps a long run of digits from overflowing.
            if (number <= arguments.size())
                number = (number * 10) + (text[end] - '0');
            ++end;
        }

        // Not an escape after all, so the percent sign stands for itself.
        if (end == i + 1)
        {
            result += '%';
            continue;
        }

        if (end < text.size() && text[end] == '%')
            ++end;

        if (number >= 1 && number <= arguments.size())
            result += arguments[number - 1];

        i = end - 1;
    }

    return result;
}

GuiString Gui::resourceStringText(StringId id)
{
    PRE(GuiResourceString::hasResource());

    GuiString result = GuiResourceString::resource().getString(id);
    POST(GuiResourceString::isInsertionString(result));
    return result;
}

GuiString Gui::formatResourceString(StringId id, const GuiStrings& arguments)
{
    PRE(GuiResourceString::hasResource());

    return substituteArguments(resourceStringText(id), arguments);
}

const GuiString& GuiResourceString::asString() const
{
    return insertionString_;
}

//////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const GuiResourceString& t)
{
    o << "GuiResourceString " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "GuiResourceString " << static_cast<const void*>(&t) << " end" << std::endl;
    return o;
}

//////////////////////////////////////////////////////////////////////

// static
bool GuiResourceString::hasResource()
{
    return pResource() != nullptr;
}

// static
AfxResourceLib& GuiResourceString::resource()
{
    PRE(hasResource());
    return *pResource();
}

// static
void GuiResourceString::resource(AfxResourceLib* pLib)
{
    PRE(pLib != nullptr);
    pResource() = pLib;
    POST(hasResource());
}

// static
void GuiResourceString::clearResource()
{
    pResource() = nullptr;
    POST(! hasResource());
}

// static
GuiResourceString::ResourcePtr& GuiResourceString::pResource()
{
    static AfxResourceLib* pResult_ = nullptr;
    return pResult_;
}

// debug-only functions //////////////////////////////////////////////

#ifndef NDEBUG

// static
bool GuiResourceString::isInsertionString(const GuiString& insertionString)
{
    // <ins str> ::= { <esc seq> | <non-percent-character> }
    // <esc seq> ::= <number esc seq> | <percent esc seq>
    // <percent esc seq> ::= <%> <%>
    // <number esc seq> ::=  <%> <number> [ <%> ]
    // <number>         ::= <leadingdigit> { <digit> }
    // <leadingdigit>   ::= <1> - <9>
    // <digit>          ::= <0> - <9>

    bool valid = true;
    for (size_t i = 0; i < insertionString.length() && valid; ++i)
    {
        if (insertionString[i] == '%')
        {
            ++i;
            if (i == insertionString.length())
                valid = false;
            else if (insertionString[i] == '%')
                ++i;
            else
            {
                valid = isdigit(insertionString[i]) && insertionString[i] != '0';

                if (valid)
                {
                    ++i;
                    while (i < insertionString.length() && isdigit(insertionString[i]))
                        ++i;

                    if (i < insertionString.length() && insertionString[i] == '%')
                        ++i;
                }
            }
        }
    }

    return valid;
}

#endif // #ifndef NDEBUG

// end debug-only functions //////////////////////////////////////////

/* End RESTRING.CPP *************************************************/
