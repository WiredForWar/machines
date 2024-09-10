#include "afx/AfxSdlApp.hpp"

#include "base/diag.hpp"
#include "device/mouse.hpp"
#include "device/keyboard.hpp"

#include "device/private/sdlkeybd.hpp"

#include "device/eventq.hpp"
#include "device/time.hpp"

#include "recorder/recorder.hpp"

#include "system/SysInfo.hpp"

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

#include <SDL.h>

AfxSdlApp::AfxSdlApp(int argc, char* argv[])
{
    AfxInvokeArgs args;
    // start from second arg
    for (int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }

    setArgs(args);
}

void AfxSdlApp::finish()
{
    // Make multiple finish calls benign.
    if (! isFinished())
    {
        finishing_ = true;
        finished_ = true;
        SDL_Quit();
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
    if (loggingEnabled_)
    {
        initLogger();
    }

    {
        SDL_version v;
        SDL_GetVersion(&v);
        spdlog::info("SDL version: {}.{}.{}", v.major, v.minor, v.patch);
    }

    {
        spdlog::info("OS: {}", getOsVersion());
    }

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

    pWindow_ = SDL_CreateWindow(
        name().c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        480, // initial width and height
        SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (pWindow_ == nullptr)
    {
        spdlog::error("Unable to create a window: {}", SDL_GetError());
        return false;
    }

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
    loggingEnabled_ = enabled;
}

void AfxSdlApp::coreLoop()
{
    bool callApp = true;

    while (! isFinished())
    {
        // Check for messages in the queue.
        SDL_Event ev;
        if (SDL_PollEvent(&ev))
        {
            // If any message other than key down or mouse move, ensure we
            // call the application
            if (ev.type != SDL_KEYDOWN && ev.type != SDL_KEYUP && ev.type != SDL_MOUSEMOTION)
            {
                callApp = true;
            }
            if (ev.type == SDL_QUIT)
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

    NEIL_STREAM("Finished app" << std::endl);
    POST(isFinished());
}

void AfxSdlApp::initLogger()
{
    std::shared_ptr<spdlog::logger> logger;
    try
    {
        constexpr int MaxSize = 1024 * 1024 * 5;
        constexpr int MaxFiles = 2;
        constexpr bool RotateOnOpen = true;

        spdlog::filename_t fileName = name();
        std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
        fileName = "logs/" + fileName + ".txt";
        logger = spdlog::rotating_logger_mt("", fileName, MaxSize, MaxFiles, RotateOnOpen);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        loggingEnabled_ = false;
    }

    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::info);
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting {} {} ({})", name(), version(), buildVersion());
}

void AfxSdlApp::dispatchEvent(SDL_Event* event)
{
    RecRecorder::instance().recordingAllowed(false);
    switch (event->type)
    {
        case SDL_WINDOWEVENT:
            break;

        case SDL_MOUSEMOTION:
            int x, y;
            SDL_GetMouseState(&x, &y);
            DevMouse::instance().position(x, y);
            break;

        case SDL_MOUSEBUTTONUP:
            dispatchMouseEvent(event, false);
            break;

        case SDL_MOUSEBUTTONDOWN:
            dispatchMouseEvent(event, true);
            break;

        case SDL_MOUSEWHEEL:
            dispatchMouseScrollEvent(event);
            break;

        case SDL_KEYUP:
            dispatchKeybrdEvent(event, false);
            break;

        case SDL_KEYDOWN:
            dispatchKeybrdEvent(event, true);
            break;

        case SDL_TEXTINPUT:
            dispatchCharEvent(event);
            break;

        case SDL_FINGERUP:
            dispatchTouchEvent(event, false);
            break;

        case SDL_FINGERDOWN:
            dispatchTouchEvent(event, true);
            break;

        default:
            std::cerr << "Unexpected event type " << event->type << std::endl;
            break;
    }
    RecRecorder::instance().recordingAllowed(true);
}

void AfxSdlApp::dispatchMouseEvent(SDL_Event* event, bool pressed)
{
    uint8_t button = event->button.button;

    if (button < 1 || button > 3)
    {
        std::cerr << "Unsupported mouse button " << button << std::endl;
        return;
    }

    // The argument is a bool so that we don't need the definition of
    // DevButtonEvent in this class's header file.  Decode the bool.
    const DevButtonEvent::Action act = (pressed) ? DevButtonEvent::PRESS : DevButtonEvent::RELEASE;

    // Get the position of the cursor at the time of the event.
    const DevMouse::Position pos = DevMouse::instance().getMessagePos();
    const int x = pos.first;
    const int y = pos.second;

    // Get the states of the modifiers keys at the time of the event.
    const Uint8* kStates = SDL_GetKeyboardState(nullptr);
    const bool shift = kStates[SDL_SCANCODE_LSHIFT] || kStates[SDL_SCANCODE_RSHIFT];
    const bool ctrl = kStates[SDL_SCANCODE_LCTRL] || kStates[SDL_SCANCODE_RCTRL];
    const bool alt = kStates[SDL_SCANCODE_LALT] || kStates[SDL_SCANCODE_RALT];

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->button.timestamp;

    // Decode wParam and lParam.
    const DevButtonEvent::ScanCode code = (button == 1) ? DevKey::LEFT_MOUSE : DevKey::RIGHT_MOUSE;
    const bool previous = false;
    const size_t repeats = 1;

    const DevButtonEvent ev(code, act, previous, shift, ctrl, alt, time, x, y, repeats);
    DevMouse::instance().wm_button(ev);
}

void AfxSdlApp::dispatchMouseScrollEvent(SDL_Event* event)
{
    PRE(event->wheel.x || event->wheel.y);

    // If direction is SDL_MOUSEWHEEL_FLIPPED the values in x and y will be opposite.
    //  Multiply by -1 to change them back.
    const DevButtonEvent::Action act = [](SDL_Event* event) {
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

    // Get the position of the cursor at the time of the event.
    const DevMouse::Position pos = DevMouse::instance().getMessagePos();
    const int x = pos.first;
    const int y = pos.second;

    // Get the states of the modifiers keys at the time of the event.
    const Uint8* kStates = SDL_GetKeyboardState(nullptr);
    const bool shift = kStates[SDL_SCANCODE_LSHIFT] || kStates[SDL_SCANCODE_RSHIFT];
    const bool ctrl = kStates[SDL_SCANCODE_LCTRL] || kStates[SDL_SCANCODE_RCTRL];
    const bool alt = kStates[SDL_SCANCODE_LALT] || kStates[SDL_SCANCODE_RALT];

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->button.timestamp;

    // Button code & whatnot
    const DevButtonEvent::ScanCode code = DevKey::MIDDLE_MOUSE;
    const bool previous = false;
    const size_t repeats = 1;

    // DISPATCH!!!
    const DevButtonEvent ev(code, act, previous, shift, ctrl, alt, time, x, y, repeats);
    DevMouse::instance().wm_button(ev);
}

void AfxSdlApp::dispatchKeybrdEvent(SDL_Event* event, bool pressed)
{
    // The argument is a bool so that we don't need the definition of
    // DevButtonEvent in this class's header file.  Decode the bool.
    const DevButtonEvent::Action act = (pressed) ? DevButtonEvent::PRESS : DevButtonEvent::RELEASE;

    // Get the position of the cursor at the time of the event.
    const DevMouse::Position pos = DevMouse::instance().getMessagePos();
    const int x = pos.first;
    const int y = pos.second;

    // Get the states of the modifiers keys at the time of the event.
    const Uint8* kStates = SDL_GetKeyboardState(nullptr);
    const bool shift = kStates[SDL_SCANCODE_LSHIFT] || kStates[SDL_SCANCODE_RSHIFT];
    const bool ctrl = kStates[SDL_SCANCODE_LCTRL] || kStates[SDL_SCANCODE_RCTRL];
    const bool alt = kStates[SDL_SCANCODE_LALT] || kStates[SDL_SCANCODE_RALT];

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->key.timestamp;

    const DevButtonEvent::ScanCode code = DevSdlKeyboard::translateScanCode(event->key.keysym.scancode);
    const bool previous = false;
    const uint16_t rpt = event->key.repeat + 1;

    const DevButtonEvent ev(code, act, previous, shift, ctrl, alt, time, x, y, rpt);
    DevSdlKeyboard::sdlInstance().wm_key(ev);
}

void AfxSdlApp::dispatchCharEvent(SDL_Event* event)
{
    // Just default these values. We're not interested in them when it's a char event.
    const bool shift = false;
    const bool ctrl = false;
    const bool alt = false;
    const bool previous = false;
    const uint16_t rpt = 1;
    const DevButtonEvent::ScanCode code = DevKey::KEY_A;
    const DevButtonEvent::Action act = DevButtonEvent::PRESS;

    // Get the position of the cursor at the time of the event.
    const DevMouse::Position pos = DevMouse::instance().getMessagePos();
    const int x = pos.first;
    const int y = pos.second;

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->text.timestamp;

    const DevButtonEvent ev(code, act, previous, shift, ctrl, alt, time, x, y, rpt, event->text.text[0]);

    DEBUG_STREAM(DIAG_NEIL, "char event " << event->text.text[0] << std::endl);

    DevSdlKeyboard::sdlInstance().wm_char(ev);
}

void AfxSdlApp::dispatchTouchEvent(SDL_Event* event, bool pressed)
{
    // The argument is a bool so that we don't need the definition of
    // DevButtonEvent in this class's header file.  Decode the bool.
    const DevButtonEvent::Action act = (pressed) ? DevButtonEvent::PRESS : DevButtonEvent::RELEASE;

    // Get the position of the cursor at the time of the event.
    const DevMouse::Position pos = DevMouse::instance().getMessagePos();
    const int x = pos.first;
    const int y = pos.second;

    // Get the states of the modifiers keys at the time of the event.
    const Uint8* kStates = SDL_GetKeyboardState(nullptr);
    const bool shift = kStates[SDL_SCANCODE_LSHIFT] || kStates[SDL_SCANCODE_RSHIFT];
    const bool ctrl = kStates[SDL_SCANCODE_LCTRL] || kStates[SDL_SCANCODE_RCTRL];
    const bool alt = kStates[SDL_SCANCODE_LALT] || kStates[SDL_SCANCODE_RALT];

    // Get the message's time.
    const double time = DevTime::instance().resolution() * event->tfinger.timestamp;

    // Decode wParam and lParam.
    const DevButtonEvent::ScanCode code = DevKey::LEFT_MOUSE;
    const bool previous = false;
    const size_t repeats = 1;

    const DevButtonEvent ev(code, act, previous, shift, ctrl, alt, time, x, y, repeats);
    DevMouse::instance().wm_button(ev);
}
