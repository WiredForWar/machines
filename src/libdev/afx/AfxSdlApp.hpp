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

    bool isLoggingEnabled() const { return loggingEnabled_; }
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

    void dispatchEvent(SDL_Event*);
    void dispatchMouseEvent(SDL_Event*, bool);
    void dispatchMouseScrollEvent(SDL_Event*);
    void dispatchKeybrdEvent(SDL_Event*, bool);
    void dispatchCharEvent(SDL_Event*);
    void dispatchTouchEvent(SDL_Event*, bool);

    bool finishing_ { false };
    bool finished_ { false };
    SDL_Window* pWindow_ = nullptr;

    bool loggingEnabled_ = false;

    // Config values needed at startup
    AfxConfiguration configuration_;
};
