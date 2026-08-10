/*
 * S R L M O D E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/Network/SerialNetworkMode.hpp"

#include "machgui/gui.hpp"
#include "machgui/StartupScreens.hpp"
#include "gui/gui.hpp"
#include "gui/Font.hpp"
#include "machgui/DropDownListBoxCreator.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/internal/strings.hpp"
#include "network/Network.hpp"
#include "network/ComPortAddress.hpp"

namespace
{
// A port setting and the text offered for it, kept together so that the two
// cannot fall out of step.
template <typename T>
struct Choice
{
    const char* label;
    T value;
};

constexpr Choice<size_t> ComPorts[] = {
    { "COM 1", 1 },
    { "COM 2", 2 },
    { "COM 3", 3 },
    { "COM 4", 4 },
};

constexpr Choice<NetComPortAddress::BaudRate> BaudRates[] = {
    { "14400", NetComPortAddress::NET_BAUD_14400 },
    { "19200", NetComPortAddress::NET_BAUD_19200 },
    { "38400", NetComPortAddress::NET_BAUD_38400 },
    { "56000", NetComPortAddress::NET_BAUD_56000 },
    { "57600", NetComPortAddress::NET_BAUD_57600 },
    { "115200", NetComPortAddress::NET_BAUD_115200 },
    { "128000", NetComPortAddress::NET_BAUD_128000 },
    { "256000", NetComPortAddress::NET_BAUD_256000 },
};

constexpr Choice<NetComPortAddress::Parity> Parities[] = {
    { "NO PARITY", NetComPortAddress::NO_PARITY },
    { "ODD", NetComPortAddress::ODD_PARITY },
    { "EVEN", NetComPortAddress::EVEN_PARITY },
    { "MARK", NetComPortAddress::MARK_PARITY },
};

constexpr Choice<NetComPortAddress::StopBits> StopBits[] = {
    { "1 BIT", NetComPortAddress::ONE_STOPBIT },
    { "1.5 BITS", NetComPortAddress::ONE5_STOPBIT },
    { "2 BITS", NetComPortAddress::TWO_STOPBITS },
};

constexpr Choice<NetComPortAddress::FlowControl> FlowControls[] = {
    { "NONE", NetComPortAddress::DTR_FLOW },
    { "DTR", NetComPortAddress::NO_FLOW },
    { "RTS/DTR", NetComPortAddress::RTSDTR_FLOW },
    { "RTS", NetComPortAddress::RTS_FLOW },
    { "XONXOFF", NetComPortAddress::XONXOFF_FLOW },
};

template <typename T, size_t N>
GuiStrings labelsOf(const Choice<T> (&choices)[N])
{
    GuiStrings labels;
    labels.reserve(N);
    for (const Choice<T>& choice : choices)
        labels.push_back(choice.label);

    return labels;
}

// What the drop down is showing, or the first choice when it is showing none.
template <typename T, size_t N>
T selectedValue(const MachGuiDropDownListBoxCreator* pDropDown, const Choice<T> (&choices)[N])
{
    const int index = pDropDown->currentIndex();
    if (index < 0 || static_cast<size_t>(index) >= N)
        return choices[0].value;

    return choices[index].value;
}

} // namespace

MachGuiSerialNetworkMode::MachGuiSerialNetworkMode(GuiDisplayable* pParent, MachGuiStartupScreens* pStartupScreens)
    : MachGuiNetworkProtocolMode(pParent, pStartupScreens)
{
    readNetworkDetails();
    TEST_INVARIANT;
}

MachGuiSerialNetworkMode::~MachGuiSerialNetworkMode()
{
    TEST_INVARIANT;
}

void MachGuiSerialNetworkMode::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void MachGuiSerialNetworkMode::setNetworkDetails()
{
    // Use the settings from the drop downs to configure network settings
    // Populate a com port object with user settings
    NetComPortAddress currentPort(selectedValue(pComPortSelecter_, ComPorts));

    currentPort.baudRate(selectedValue(pBaudRateSelecter_, BaudRates));
    currentPort.parity(selectedValue(pParitySelecter_, Parities));
    currentPort.stopBits(selectedValue(pStopBitsSelecter_, StopBits));
    currentPort.flowControl(selectedValue(pFlowSelecter_, FlowControls));

    // Set the current com port settings
    //  NetNetwork::instance().comPortAddress( currentPort );

    NetNetwork::instance().initialiseConnection();
}

// virtual
bool MachGuiSerialNetworkMode::validNetworkDetails(bool /*isHost*/)
{
    return true;
}

#define SNM_MINX 62
#define SNM_MINY 309
#define SNM_WIDTH 82
#define SNM_MINX2 SNM_MINX + SNM_WIDTH + 15
#define SNM_DEPTH 30
#define SNM_MINY2 SNM_MINY + SNM_DEPTH
#define SNM_MINY3 SNM_MINY + (SNM_DEPTH * 2)

// virtual
void MachGuiSerialNetworkMode::readNetworkDetails()
{
    GuiResourceString comPortHeading(IDS_MENU_COMPORT);
    GuiBmpFont font(Gui::getFont(MachGui::Menu::smallFontLight()));
    const int textHeight = font.height() + 2 * MachGui::menuScaleFactor();

    MachGuiMenuText* pComPortText = new MachGuiMenuText(
        &startupScreens(),
        Gui::Box(Gui::Coord(SNM_MINX, SNM_MINY), font.horizontalAdvance(comPortHeading.asString()), textHeight),
        IDS_MENU_COMPORT,
        MachGui::Menu::smallFontLight());

    pComPortSelecter_ = new MachGuiDropDownListBoxCreator(
        &parent(),
        &startupScreens(),
        Gui::Coord(SNM_MINX, SNM_MINY + textHeight),
        SNM_WIDTH,
        true,
        true);
    pComPortSelecter_->setAvailText(labelsOf(ComPorts));

    GuiResourceString baudHeading(IDS_MENU_BAUD);

    MachGuiMenuText* pBaudText = new MachGuiMenuText(
        &startupScreens(),
        Gui::Box(Gui::Coord(SNM_MINX, SNM_MINY2), font.horizontalAdvance(baudHeading.asString()), textHeight),
        IDS_MENU_BAUD,
        MachGui::Menu::smallFontLight());

    pBaudRateSelecter_ = new MachGuiDropDownListBoxCreator(
        &parent(),
        &startupScreens(),
        Gui::Coord(SNM_MINX, SNM_MINY2 + textHeight),
        SNM_WIDTH,
        true,
        true);
    pBaudRateSelecter_->setAvailText(labelsOf(BaudRates));

    GuiResourceString parityHeading(IDS_MENU_PARITY);

    MachGuiMenuText* pParityText = new MachGuiMenuText(
        &startupScreens(),
        Gui::Box(Gui::Coord(SNM_MINX2, SNM_MINY2), font.horizontalAdvance(parityHeading.asString()), textHeight),
        IDS_MENU_PARITY,
        MachGui::Menu::smallFontLight());

    pParitySelecter_ = new MachGuiDropDownListBoxCreator(
        &parent(),
        &startupScreens(),
        Gui::Coord(SNM_MINX2, SNM_MINY2 + textHeight),
        SNM_WIDTH,
        true,
        true);
    pParitySelecter_->setAvailText(labelsOf(Parities));

    GuiResourceString stopBitsHeading(IDS_MENU_STOPBITS);

    MachGuiMenuText* pStopBitsText = new MachGuiMenuText(
        &startupScreens(),
        Gui::Box(Gui::Coord(SNM_MINX, SNM_MINY3), font.horizontalAdvance(stopBitsHeading.asString()), textHeight),
        IDS_MENU_STOPBITS,
        MachGui::Menu::smallFontLight());

    pStopBitsSelecter_ = new MachGuiDropDownListBoxCreator(
        &parent(),
        &startupScreens(),
        Gui::Coord(SNM_MINX, SNM_MINY3 + textHeight),
        SNM_WIDTH,
        true,
        true);
    pStopBitsSelecter_->setAvailText(labelsOf(StopBits));

    GuiResourceString flowHeading(IDS_MENU_FLOW);

    MachGuiMenuText* pFlowText = new MachGuiMenuText(
        &startupScreens(),
        Gui::Box(Gui::Coord(SNM_MINX2, SNM_MINY3), font.horizontalAdvance(flowHeading.asString()), textHeight),
        IDS_MENU_FLOW,
        MachGui::Menu::smallFontLight());

    pFlowSelecter_ = new MachGuiDropDownListBoxCreator(
        &parent(),
        &startupScreens(),
        Gui::Coord(SNM_MINX2, SNM_MINY3 + textHeight),
        SNM_WIDTH,
        true,
        true);
    pFlowSelecter_->setAvailText(labelsOf(FlowControls));
}

std::ostream& operator<<(std::ostream& o, const MachGuiSerialNetworkMode& t)
{
    o << "MachGuiSerialNetworkMode " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiSerialNetworkMode " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End SRLMODE.CPP **************************************************/
