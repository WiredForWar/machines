#include "afx/AfxSdlApp.hpp"

#include "base/Diag.hpp"
#include "crashdump/CrashDump.hpp"
#include "device/Input.hpp"
#include "device/Keyboard.hpp"
#include "device/Mouse.hpp"
#include "device/SdlKeyCodes.hpp"
#include "device/Time.hpp"

#include "system/SysInfo.hpp"

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <charconv>
#include <optional>
#include <vector>

AfxSdlApp::AfxSdlApp(int argc, char* argv[])
{
    AfxInvokeArgs args;
    // start from second arg
    for (int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }

    setArgs(args);

    // A run nobody is sitting in front of must not stop on a dialog box. A run
    // driven over the test server is one of those by definition.
    if (args.contains("--no-assert-dialog") || args.contains("--test-server")
        || args.value("--test-server").has_value())
    {
        Base::abortWithoutADialog();
    }
}

void AfxSdlApp::finish()
{
    // Make multiple finish calls benign.
    if (! isFinished())
    {
        finishing_ = true;
        finished_ = true;
    }
}

bool AfxSdlApp::isFinished() const
{
    return finished_;
}

void AfxSdlApp::testPrint(const char*) const
{
}

bool AfxSdlApp::OSStartup()
{
    initLogger();

    {
        const std::string& packaging = getPackagingInfo();
        if (packaging.empty())
        {
            spdlog::info("OS: {}", getOsVersion());
        }
        else
        {
            spdlog::info("OS: {} ({})", getOsVersion(), packaging);
        }
    }

    {
        const int v = SDL_GetVersion();
        spdlog::info(
            "SDL version: {}.{}.{}",
            SDL_VERSIONNUM_MAJOR(v),
            SDL_VERSIONNUM_MINOR(v),
            SDL_VERSIONNUM_MICRO(v));
    }

    // Report pointer travel as the device measures it, with no system pointer
    // acceleration curve and no system pointer speed applied. Each system shapes those
    // differently, so leaving them in makes the same movement of the hand mean
    // different things on different systems. Both are SDL's defaults; ask for them
    // anyway, because aiming depends on them.
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SYSTEM_SCALE, "0");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SPEED_SCALE, "1.0");

    // Create window
    spdlog::info("Initializing SDL...");
    SDL_Init(SDL_INIT_VIDEO);

    return recreateWindow();
}

bool AfxSdlApp::recreateWindow()
{
    if (pWindow_)
        SDL_DestroyWindow(pWindow_);

    {
        bool doubleBuffer = true;
        int buffers = configuration_.getConfig().multisampleBuffers;
        int samples = configuration_.getConfig().multisampleSamples;

        spdlog::info("Double buffer: {}", doubleBuffer);
        spdlog::info("Multisample buffers/samples: {}/{}", buffers, samples);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, doubleBuffer);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, buffers);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);
    }

    // Create a hidden window: the real resolution and the fullscreen
    // state are only known later, when RenDisplay::useMode() applies them and
    // shows the window.
    //
    // Sixteen depth bits is what SDL asks for when nobody says otherwise, and a
    // quarter of the resolution every desktop driver has offered for decades: at
    // sixteen the furthest geometry rounds onto the value the buffer was cleared
    // to and is not drawn at all. The attributes are read where the window picks
    // its pixel format rather than where the context is made, so the depth has to
    // be asked for here. A pixel format that deep may not exist everywhere, hence
    // the second attempt at what SDL would have chosen by itself.
    for (const int depthBits : {24, 16})
    {
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthBits);

        pWindow_ = SDL_CreateWindow(
            name().c_str(),
            640,
            480, // initial width and height
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

        if (pWindow_ != nullptr)
            break;

        spdlog::warn("Unable to create a window with {} depth bits: {}", depthBits, SDL_GetError());
    }

    if (pWindow_ == nullptr)
    {
        spdlog::error("Unable to create a window: {}", SDL_GetError());
        return false;
    }

    SDL_SetWindowPosition(pWindow_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // Unlike SDL2, SDL3 does not deliver SDL_EVENT_TEXT_INPUT unless text
    // input is explicitly started.
    SDL_StartTextInput(pWindow_);

    // Can this method fail?
    return true;
}

void AfxSdlApp::OSShutdown()
{
    SDL_DestroyWindow(pWindow_);
    SDL_Quit();
}

void AfxSdlApp::setLoggingEnabled(bool enabled)
{
    logFileEnabled_ = enabled;
}

void AfxSdlApp::coreLoop()
{
    bool callApp = true;

    startWatchdog();

    while (! isFinished())
    {
        // One relaxed store, telling the watchdog the loop is still turning.
        CrashDump::heartbeat();

        // Check for messages in the queue.
        SDL_Event ev;
        if (SDL_PollEvent(&ev))
        {
            // If any message other than key down or mouse move, ensure we
            // call the application
            if (ev.type != SDL_EVENT_KEY_DOWN && ev.type != SDL_EVENT_KEY_UP && ev.type != SDL_EVENT_MOUSE_MOTION)
            {
                callApp = true;
            }
            if (ev.type == SDL_EVENT_QUIT)
            {
                // When should this be set?  I assume that there could be outstanding
                // messages in the queue when PostQuitMessage is called.  Therefore,
                // the program may be viewed as still running until SDL_QUIT is
                // processed and GetMessage returns false.
                finished_ = true;
            }
            else
            {
                dispatchEvent(&ev);
            }
            // if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            // onResize(ev.window.data1, ev.window.data2);

            // If we haven't yet decided to call the app, don't call it if there
            // are any keyboard messages still in the queue
            if (! callApp)
            {
                callApp = ! SDL_PollEvent(&ev);

                if (! callApp)
                {
                    dispatchEvent(&ev);
                }
            }
        }
        else
        {
            callApp = true; // No messages waiting
        }

        // Let the client app do its thing.
        // Note: this main loop will continue until SDL_QUIT is processed,
        // however, the client's loopCycle() method will not be called
        // after finishing_ is set by AfxWin95App::finish().
        if (callApp && !finished_ && !finishing_)
        {
            loopCycle();
            callApp = false;
        }
    }

    CrashDump::stopWatchdog();

    NEIL_STREAM("Finished app" << std::endl);
    POST(isFinished());
}

void AfxSdlApp::startWatchdog()
{
    // The command line wins so that one session can be given a different
    // setting without editing anything, but the configuration file is where it
    // normally lives: the people who hit a hang start the game from a desktop
    // entry or a packaged launcher, where passing a flag is not something they
    // can reasonably be asked to do.
    int timeout = configuration_.getConfig().watchdogTimeout;

    if (const std::optional<std::string_view> given = invokeArgs().value("--watchdog-timeout"))
    {
        int parsed{};
        const std::from_chars_result result = std::from_chars(given->data(), given->data() + given->size(), parsed);

        if (result.ec == std::errc() && parsed >= 0)
        {
            timeout = parsed;
        }
        else
        {
            spdlog::warn("Ignoring --watchdog-timeout={}: expected a whole number of seconds", *given);
        }
    }

    if (timeout <= 0)
    {
        spdlog::info("Hang watchdog disabled");
        return;
    }

    spdlog::info("Hang watchdog armed at {} seconds", timeout);
    CrashDump::startWatchdog(std::chrono::seconds(timeout));
}

void AfxSdlApp::initLogger()
{
    const bool logToConsole = invokeArgs().contains("--log-to-console");

    std::shared_ptr<spdlog::sinks::sink> fileSink;
    if (logFileEnabled_)
    {
        try
        {
            constexpr int MaxSize = 1024 * 1024 * 5;
            constexpr int MaxFiles = 2;
            constexpr bool RotateOnOpen = true;

            spdlog::filename_t fileName = name();
            std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
            fileName = "logs/" + fileName + ".txt";
            fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(fileName, MaxSize, MaxFiles, RotateOnOpen);
            fileSink->set_level(spdlog::level::debug);
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            std::cerr << "Log init failed: " << ex.what() << std::endl;
            logFileEnabled_ = false;
        }
    }

    std::shared_ptr<spdlog::sinks::sink> consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_st>();
    consoleSink->set_level((logToConsole || !fileSink) ? spdlog::level::debug : spdlog::level::warn);

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(consoleSink);
    if (fileSink)
        sinks.push_back(fileSink);

    auto logger = std::make_shared<spdlog::logger>(std::string(), sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::info);
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting {} {} ({})", name(), version(), buildVersion());
    if (!buildInfo().empty())
    {
        spdlog::info("Build: {}", buildInfo());
    }
}

void AfxSdlApp::dispatchEvent(const SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            Device::submitFocusLost();
            break;

        case SDL_EVENT_MOUSE_MOTION:
            Device::submitPointerPosition(static_cast<int>(event->motion.x), static_cast<int>(event->motion.y));
            Device::submitPointerMotion(event->motion.xrel, event->motion.yrel);
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            dispatchMouseButtonEvent(event, false);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            dispatchMouseButtonEvent(event, true);
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            dispatchMouseScrollEvent(event);
            break;

        case SDL_EVENT_KEY_UP:
            dispatchKeyboardEvent(event, false);
            break;

        case SDL_EVENT_KEY_DOWN:
            dispatchKeyboardEvent(event, true);
            break;

        case SDL_EVENT_TEXT_INPUT:
            dispatchCharEvent(event);
            break;

        case SDL_EVENT_FINGER_UP:
            dispatchTouchEvent(event, false);
            break;

        case SDL_EVENT_FINGER_DOWN:
            dispatchTouchEvent(event, true);
            break;

        default:
            break;
    }
}

namespace
{

struct HeldModifiers
{
    bool shift{};
    bool ctrl{};
    bool alt{};
};

// The modifiers the game believes are held. Taken from DevKeyboard rather than
// from SDL, so a submitted key press counts on the same terms as a real one.
HeldModifiers heldModifiers()
{
    return {
        .shift = DevKeyboard::instance().shiftPressed(),
        .ctrl = DevKeyboard::instance().ctrlPressed(),
        .alt = DevKeyboard::instance().altPressed(),
    };
}

} // namespace

void AfxSdlApp::dispatchMouseButtonEvent(const SDL_Event* event, bool pressed)
{
    const DevButtonEvent::ScanCode code = [](uint8_t button) {
        switch (button)
        {
            case 1:
                return Device::KeyCode::MOUSE_LEFT;
            case 3:
                return Device::KeyCode::MOUSE_RIGHT;
            case 4:
                return Device::KeyCode::MOUSE_EXTRA1;
            case 5:
                return Device::KeyCode::MOUSE_EXTRA2;
            case 6:
                return Device::KeyCode::MOUSE_EXTRA3;
            case 7:
                return Device::KeyCode::MOUSE_EXTRA4;
            case 8:
                return Device::KeyCode::MOUSE_EXTRA5;
            case 9:
                return Device::KeyCode::MOUSE_EXTRA6;
            case 10:
                return Device::KeyCode::MOUSE_EXTRA7;
            case 11:
                return Device::KeyCode::MOUSE_EXTRA8;

            case 2:
            default:
                break;
        }
        return Device::KeyCode::UNKNOWN;
    }(event->button.button);

    if (code == Device::KeyCode::UNKNOWN)
    {
        std::cerr << "Unsupported mouse button " << static_cast<int>(event->button.button) << std::endl;
        return;
    }

    // The argument is a bool so that we don't need the definition of
    // DevButtonEvent in this class's header file.  Decode the bool.
    const DevButtonEvent::Action act = (pressed) ? DevButtonEvent::PRESS : DevButtonEvent::RELEASE;

    // The event carries the position it happened at.
    const int x = static_cast<int>(event->button.x);
    const int y = static_cast<int>(event->button.y);

    const HeldModifiers modifiers = heldModifiers();

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->button.timestamp;

    // Decode wParam and lParam.
    const bool previous = false;
    const size_t repeats = 1;

    Device::submitButtonEvent(
        DevButtonEvent(code, act, previous, modifiers.shift, modifiers.ctrl, modifiers.alt, time, x, y, repeats));
}

void AfxSdlApp::dispatchMouseScrollEvent(const SDL_Event* event)
{
    PRE(event->wheel.x || event->wheel.y);

    // If direction is SDL_MOUSEWHEEL_FLIPPED the values in x and y will be opposite.
    //  Multiply by -1 to change them back.
    const DevButtonEvent::Action act = [](const SDL_Event* event) {
        const int multiplier = (event->wheel.direction == SDL_MOUSEWHEEL_NORMAL) ? 1 : -1;
        if (event->wheel.x)
        {
            return (event->wheel.x * multiplier > 0) ? DevButtonEvent::SCROLL_RIGHT : DevButtonEvent::SCROLL_LEFT;
        }
        else
        {
            return (event->wheel.y * multiplier > 0) ? DevButtonEvent::SCROLL_UP : DevButtonEvent::SCROLL_DOWN;
        }
    }(event);

    // The event carries the position it happened at.
    const int x = static_cast<int>(event->wheel.mouse_x);
    const int y = static_cast<int>(event->wheel.mouse_y);

    const HeldModifiers modifiers = heldModifiers();

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->wheel.timestamp;

    // Button code & whatnot
    const DevButtonEvent::ScanCode code = Device::KeyCode::MOUSE_MIDDLE;
    const bool previous = false;
    const size_t repeats = 1;

    Device::submitButtonEvent(
        DevButtonEvent(code, act, previous, modifiers.shift, modifiers.ctrl, modifiers.alt, time, x, y, repeats));
}

void AfxSdlApp::dispatchKeyboardEvent(const SDL_Event* event, bool pressed)
{
    // The argument is a bool so that we don't need the definition of
    // DevButtonEvent in this class's header file.  Decode the bool.
    const DevButtonEvent::Action act = (pressed) ? DevButtonEvent::PRESS : DevButtonEvent::RELEASE;

    // A key event happens wherever the pointer already is.
    const DevMouse::Position pos = DevMouse::instance().position();
    const int x = pos.x;
    const int y = pos.y;

    // Use the event's own modifier state rather than SDL_GetKeyboardState which
    // can carry stale modifier flags from before the game window gained focus.
    const SDL_Keymod mod = event->key.mod;
    const bool shift = (mod & (SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT)) != 0;
    const bool ctrl = (mod & (SDL_KMOD_LCTRL | SDL_KMOD_RCTRL)) != 0;
    const bool alt = (mod & (SDL_KMOD_LALT | SDL_KMOD_RALT)) != 0;

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->key.timestamp;

    const DevButtonEvent::ScanCode code = Device::codeFromSdlScanCode(event->key.scancode);
    const bool previous = false;
    const uint16_t rpt = event->key.repeat + 1;

    Device::submitButtonEvent(DevButtonEvent(code, act, previous, shift, ctrl, alt, time, x, y, rpt));
}

void AfxSdlApp::dispatchCharEvent(const SDL_Event* event)
{
    // Just default these values. We're not interested in them when it's a char event.
    const bool shift = false;
    const bool ctrl = false;
    const bool alt = false;
    const bool previous = false;
    const uint16_t rpt = 1;
    const DevButtonEvent::ScanCode code = Device::KeyCode::KEY_A;
    const DevButtonEvent::Action act = DevButtonEvent::PRESS;

    // A char event happens wherever the pointer already is.
    const DevMouse::Position pos = DevMouse::instance().position();
    const int x = pos.x;
    const int y = pos.y;

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->text.timestamp;

    const DevButtonEvent ev(code, act, previous, shift, ctrl, alt, time, x, y, rpt, event->text.text[0]);

    DEBUG_STREAM(DIAG_NEIL, "char event " << event->text.text[0] << std::endl);

    Device::submitCharEvent(ev);
}

void AfxSdlApp::dispatchTouchEvent(const SDL_Event* event, bool pressed)
{
    // The argument is a bool so that we don't need the definition of
    // DevButtonEvent in this class's header file.  Decode the bool.
    const DevButtonEvent::Action act = (pressed) ? DevButtonEvent::PRESS : DevButtonEvent::RELEASE;

    // A contact reports a position normalised to the window, which nothing here
    // converts yet, so this still takes the pointer's.
    const DevMouse::Position pos = DevMouse::instance().position();
    const int x = pos.x;
    const int y = pos.y;

    const HeldModifiers modifiers = heldModifiers();

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->tfinger.timestamp;

    // Decode wParam and lParam.
    const DevButtonEvent::ScanCode code = Device::KeyCode::MOUSE_LEFT;
    const bool previous = false;
    const size_t repeats = 1;

    Device::submitButtonEvent(
        DevButtonEvent(code, act, previous, modifiers.shift, modifiers.ctrl, modifiers.alt, time, x, y, repeats));
}
