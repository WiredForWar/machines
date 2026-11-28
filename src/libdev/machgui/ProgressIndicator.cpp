#include "ProgressIndicator.hpp"
#include "machgui/ProgressBarFill.hpp"
#include "machgui/ProgressBarRenderer.hpp"

#include "machlog/Messaging/Network.hpp"
#include "render/Device.hpp"
#include "render/Display.hpp"
#include "render/Painter.hpp"
#include "system/WindowsAPI.hpp"

namespace MachGui
{

ProgressIndicator::ProgressIndicator(Gui::Box area, Gui::Colour color)
    : upperLimit_(1.0)
    , area_(area)
    , color_(color)
{
}

std::size_t ProgressIndicator::reportImpl(std::size_t done, std::size_t maxDone)
{
    const int displayWidth = progressBarFillWidth(area_.width(), done, maxDone, lowerLimit_, upperLimit_);
    if (displayWidth == lastWidth_)
        return 0;

    const double minx = area_.left();
    const double miny = area_.top();
    const double width = area_.width();
    const double height = area_.height();
    RenSurface frontBuf = RenDevice::current()->frontSurface();
    const auto paint = [&]() {
        // Submit an immediate buffer before swapping, rather than keeping its
        // painter alive across the swap and submitting both passes afterwards.
        Ren::Painter painter(frontBuf);
        drawProgressBarFill(painter, Ren::Rect(minx, miny, width, height), displayWidth, color_);
    };
    paint();
    RenDevice::current()->flushCommandBuffer();
    RenDevice::current()->display()->flipBuffers();
    // For double buffering do it twice to prevent bar from blinking
    paint();
    RenDevice::current()->flushCommandBuffer();
    // If the game session has come out prematurely then the network connection may have been reset.

    // Incoming net message can reference something which is not-loaded-yet (e.g. a Race)
    // Comment out the net updates for now.
    // if (MachLogNetwork::instance().isNetworkGame())
    // {
    //     MachLogNetwork::instance().update();
    //     SysWindowsAPI::sleep(0);
    //     SysWindowsAPI::peekMessage();
    //     // network game _may_ have come out on update above.
    //     /*              if( MachLogNetwork::instance().isNetworkGame() and
    //     MachLogNetwork::instance().node().lastPingAllTime() > 1 )
    //     {
    //         MachLogNetwork::instance().node().pingAll();
    //     }*/
    // }
    lastWidth_ = displayWidth;

    return static_cast<double>(maxDone) / 50.0;
}

void ProgressIndicator::setLimits(double lower, double upper)
{
    lowerLimit_ = lower;
    upperLimit_ = upper;
}

} // namespace MachGui
