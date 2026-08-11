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

} // namespace Gui

class GuiResourceString
// memberwise cannonical
{
public:
    // loading string with id from current resource file
    GuiResourceString(Gui::StringId id);
    // PRE( hasResource() );

    // as above, replacing all occurrences of %1 with insert
    GuiResourceString(Gui::StringId id, const GuiString& insert);
    // PRE( hasResource() );

    // as above, replacing all occurrences of %1 with insert[ 0 ],
    // %2 with insert[ 1 ] etc
    GuiResourceString(Gui::StringId id, const GuiStrings& insert);
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
    static GuiString map_Id_to_string(Gui::StringId id);
    // PRE( hasResource() );
    // POST( isInsertionString( result ) );

    void insert(const GuiStrings&);

    GuiString insertionString_;

    friend std::ostream& operator<<(std::ostream& o, const GuiResourceString& t);
};

//////////////////////////////////////////////////////////////////////

#endif

/* End RESTRING.HPP *************************************************/
