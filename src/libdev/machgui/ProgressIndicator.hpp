#pragma once

#include "base/IProgressReporter.hpp"

#include "gui/gui.hpp"

namespace MachGui
{

class ProgressIndicator : public IProgressReporter
{
public:
    ProgressIndicator(Gui::Box area, Gui::Colour color);

    void setLimits(double lower, double upper);

protected:
    std::size_t reportImpl(std::size_t done, std::size_t maxDone) override;

private:
    double lowerLimit_{};
    double upperLimit_{};
    std::size_t lastDone_{};
    Gui::Box area_{};
    Gui::Colour color_ {};
};

} // namespace MachGui
