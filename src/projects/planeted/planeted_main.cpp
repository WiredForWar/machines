#include "afx/AfxSdlApp.hpp"

#include <SDL.h>

int main(int argc, char* argv[])
{
    try
    {
        AfxSdlApp::sdlInstance().initialise(argc, argv);

        auto const runResult = AfxApp::abstractInstance().run();

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
