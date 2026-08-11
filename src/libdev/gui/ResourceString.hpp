/*
 * R E S T R I N G . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
 * GuiResourceString
 * Supports loading of internationalized strings from a resource file.
 */

#ifndef _GUI_RESTRING_HPP
#define _GUI_RESTRING_HPP

#include "gui/StringId.hpp"

#include <format>
#include <string>
#include <string_view>
#include <vector>

//////////////////////////////////////////////////////////////////////

class AfxResourceLib;

// TBD: support for extended character sets, double-byte character sets, Unicode etc.

using GuiString = std::string;
using GuiStrings = std::vector<GuiString>;

namespace Gui
{

// Copies text, replacing every %1, %2 ... escape with the argument of that number
// and every %% with a single percent sign. Numbering starts at one, an escape may
// be closed with a second percent sign so that it can be followed by a digit, and
// an escape naming an argument that was not supplied yields nothing. Arguments are
// copied out verbatim, so a percent sign within one is never an escape.
GuiString substituteArguments(std::string_view text, const GuiStrings& arguments);

// The text held for id in the current resource file.
GuiString resourceStringText(StringId id);
// PRE( GuiResourceString::hasResource() );

// The text held for id, with its escapes replaced by the arguments.
GuiString formatResourceString(StringId id, const GuiStrings& arguments);
// PRE( GuiResourceString::hasResource() );

// As above, for a number of arguments known where the call is written. Each one
// is converted the way std::format would print it on its own, so a count or a
// name goes straight in and does not have to be spelled into a buffer first.
template <class... Args>
GuiString formatResourceString(StringId id, const Args&... arguments)
{
    return formatResourceString(id, GuiStrings{ std::format("{}", arguments)... });
}

} // namespace Gui

class GuiResourceString
// memberwise cannonical
{
public:
    // loading string with id from current resource file
    GuiResourceString(Gui::StringId id);
    // PRE( hasResource() );

    const GuiString& asString() const;

    ////////////////////////////

    static bool isInsertionString(const GuiString&);
    // <ins str> ::= { <esc seq> | <non-percent-character> }
    // <esc seq> ::= <number esc seq> | <percent esc seq>
    // <percent esc seq> ::= <%> <%>
    // <number esc seq> ::=  <%> <number> [ <%> ]
    // <number>         ::= <leadingdigit> { <digit> }
    // <leadingdigit>   ::= <1> - <9>
    // <digit>          ::= <0> - <9>

    static bool hasResource();

    static AfxResourceLib& resource();
    // PRE( hasResource() );

    static void resource(AfxResourceLib* pLib);
    // PRE( pLib != NULL );
    // POST( hasResource() );

    static void clearResource();
    // POST( not hasResource() )

    ////////////////////////////

private:
    using ResourcePtr = AfxResourceLib*;
    static ResourcePtr& pResource();
    GuiString insertionString_;

    friend std::ostream& operator<<(std::ostream& o, const GuiResourceString& t);
};

//////////////////////////////////////////////////////////////////////

#endif

/* End RESTRING.HPP *************************************************/
