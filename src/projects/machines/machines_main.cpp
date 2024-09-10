#include "sdlapp.hpp"

#include "MachinesVersion.hpp"

#include "spdlog/spdlog.h"

#include <SDL.h>

int main(int argc, char* argv[])
{
    SDLApp app(argc, argv);
    app.setAppName("Machines");
    app.setVersion(machinesVersion());
    app.setBuildVersion(machinesBuildVersion());
    app.setLoggingEnabled(true);

    try
    {
        auto const runResult = app.run();

        return runResult;
    }
    catch (std::exception const& e)
    {
        if (app.isLoggingEnabled())
        {
            spdlog::critical("Fatal error (unhandled exception). Message: {}", e.what());
        }
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
