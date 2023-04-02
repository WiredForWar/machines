#pragma once

#include "afx/app.hpp"
#include "AfxConfiguration.hpp"

#include <SDL.h>

class AfxSdlApp : public AfxApp
{
public:
    static AfxSdlApp& sdlInstance();

    AfxSdlApp() = default;

    ~AfxSdlApp() override = default;

    SDL_Window* window() { return pWindow_; }

    void finish() override;
    bool isFinished() const override;

    void testPrint(const char*) const override;

protected:
    bool OSStartup() override;
    void OSShutdown() override;

    void setAppName(const std::string& name);
    void setAppVersion(const std::string& version);
    void setAppBuildVersion(const std::string& buildVersion);

    void setLoggingEnabled(bool enabled);

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
    void initialise(int argc, char* argv[]);

    friend int main(int, char*[]);

    bool finishing_ { false };
    bool finished_ { false };
    SDL_Window* pWindow_;

    std::string appName_;
    std::string version_;
    std::string buildVersion_;
    bool loggingEnabled_ = false;

    // Config values needed at startup
    AfxConfiguration configuration_;
};
