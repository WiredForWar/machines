#include "base/IProgressReporter.hpp"

#include "gui/gui.hpp"

namespace MachGui
{

class ProgressIndicator : public IProgressReporter
{
public:
    ProgressIndicator(Gui::Box area, Gui::Colour color);

    size_t report(size_t done, size_t maxDone) override;

    void setLimits(double lower, double upper);

private:
    double lowerLimit_ = 0;
    double upperLimit_ = 0;
    size_t lastDone_;
    Gui::Box area_;
    Gui::Colour color_ {};
};

} // namespace MachGui
