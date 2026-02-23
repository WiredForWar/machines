#pragma once

#include "gui/displaya.hpp"
#include "gui/font.hpp"
#include "machgui/SingleLineEditBox.hpp"
#include "utility/CallbackHandle.hpp"

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace System
{

class IConsole;

} // namespace System

class MachGuiConsoleDropDown : public GuiDisplayable
{
public:
    explicit MachGuiConsoleDropDown(GuiDisplayable* parent);
    ~MachGuiConsoleDropDown() override;

    void setConsole(System::IConsole* console);
    void setViewportSize(Gui::Size size);

    void submit();

    void doDisplay() override;

    void focusInput();
    void blurInput();
    void updateInput();

    void setOpen(bool open);
    bool isOpen() const;
    void toggle();

    bool doHandleKeyEvent(const GuiKeyEvent& event) override;

private:
    std::string inputText() const;

    MachGuiSingleLineEditBox* inputBox() const;
    void doLayout();
    void prerenderOutput();
    void setOutput(const std::vector<std::string>& lines);
    void updateOutputFromConsole();
    void updatePromptSurface(std::string_view promptText);

    System::IConsole *pConsole_{};
    Utils::CallbackHandleUPtr outputListenerHandle_{};
    std::unique_ptr<MachGuiSingleLineEditBox> inputBox_;
    GuiBitmap promptBmp_;
    Gui::Coord promptCoord_{};
    GuiBmpFont inputFont_{};

    Gui::Coord contentCoord_{};
    std::optional<GuiBitmap> consoleOutput_;

    std::vector<std::string> outputLines_{};

    int borderThickness_{};
    int inputAreaHeight_{};
    int inputPadding_{};
    bool open_{};
};
