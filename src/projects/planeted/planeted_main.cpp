#include "sdlapp.hpp"

#include "MachinesVersion.hpp"
#include "crashdump/CrashDump.hpp"
#include "crashdump/CrashTest.hpp"

#include <SDL3/SDL.h>
// The SDL2main library is gone in SDL3; the header-only SDL_main.h provides
// the platform entry point (e.g. WinMain for windowed MSVC builds).
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[])
{
    // Before anything else, so that a failure during start-up is reported too.
    // Reports go where the log does, which is a directory every packaging
    // channel already places in the user's own data directory.
    CrashDump::initialize("logs");
    // The editor carries no product version of its own worth reporting, so the
    // build is the only thing that identifies which one a report came from.
    CrashDump::setApplicationInfo("Machines Planet Editor", {}, machinesBuildVersion());

    // Ahead of SDL, so that provoking a crash needs no window, no graphics
    // device and no game data, and can therefore run on a build machine.
    CrashDump::runCrashTestIfRequested(argc, argv);

    SDLApp app(argc, argv);
    app.setAppName("Machines Planet Editor");
    app.setVersion("v0.00.01");
    app.setBuildVersion(machinesBuildVersion());
    app.setBuildInfo(machinesBuildInfo());

    try
    {
        auto const runResult = app.run();

        return runResult;
    }
    catch (std::exception const& e)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Crash",
            (std::string { "A fatal error (unhandled exception) has occurred, application will be terminated. Msg:\n" }
             + e.what())
                .c_str(),
            nullptr);

        return -1;
    }
    catch (...)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Crash",
            "A fatal error (unhandled exception) has occurred, application will be terminated.",
            nullptr);

        return -1;
    }
}
