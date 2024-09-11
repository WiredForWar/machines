#pragma once

#include "afx/app.hpp"
#include "AfxConfiguration.hpp"

#include <SDL.h>

class AfxSdlApp : public AfxApp
{
public:
    AfxSdlApp(int argc, char* argv[]);

    ~AfxSdlApp() override = default;

    bool recreateWindow();
    SDL_Window* window() { return pWindow_; }

    void finish() override;
    bool isFinished() const override;

    void testPrint(const char*) const override;

    bool isLoggingEnabled() const { return logFileEnabled_; }
    void setLoggingEnabled(bool enabled);

protected:
    bool OSStartup() override;
    void OSShutdown() override;

private:
    // These are called only by this class from the run method.
    // The default implementations call the client and OS specific
    // start-up and shutdown methods.
    void coreLoop() override;

    void initLogger();

    void dispatchEvent(const SDL_Event* event);
    void dispatchMouseButtonEvent(const SDL_Event* event, bool pressed);
    void dispatchMouseScrollEvent(const SDL_Event* event);
    void dispatchKeyboardEvent(const SDL_Event* event, bool pressed);
    void dispatchCharEvent(const SDL_Event* event);
    void dispatchTouchEvent(const SDL_Event* event, bool pressed);

    bool finishing_ { false };
    bool finished_ { false };
    SDL_Window* pWindow_ = nullptr;

    bool logFileEnabled_ = false;

    // Config values needed at startup
    AfxConfiguration configuration_;
};
