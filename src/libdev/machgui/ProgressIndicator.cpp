#include "ProgressIndicator.hpp"

#include "machlog/network.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "system/winapi.hpp"

namespace MachGui
{

ProgressIndicator::ProgressIndicator(Gui::Box area, Gui::Colour color)
    : upperLimit_(1.0)
    , area_(area)
    , color_(color)
{
}

size_t ProgressIndicator::report(size_t done, size_t maxDone)
{
    if (done == lastDone_)
        return 0;

    const double minx = area_.left();
    const double miny = area_.top();
    const double width = area_.width();
    const double height = area_.height();
    const double limitRange = upperLimit_ - lowerLimit_;
    const double percentComplete = static_cast<double>(done) / static_cast<double>(maxDone) * limitRange + lowerLimit_;
    const double displayWidth = std::min((percentComplete * width) + 5, width);

    RenDevice::current()->frontSurface().filledRectangle(Ren::Rect(minx, miny, displayWidth, height), color_);
    RenDevice::current()->display()->flipBuffers();
    // For double buffering do it twice to prevent bar from blinking
    RenDevice::current()->frontSurface().filledRectangle(Ren::Rect(minx, miny, displayWidth, height), color_);
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
    lastDone_ = done;

    return static_cast<double>(maxDone) / 50.0;
}

void ProgressIndicator::setLimits(double lower, double upper)
{
    lowerLimit_ = lower;
    upperLimit_ = upper;
}

} // namespace MachGui
