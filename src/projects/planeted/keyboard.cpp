#include "planeted/sdlapp.hpp"
#include "planeted/cameras.hpp"

#include <iostream>
#include <iomanip>

#include "base/base.hpp"

#include "system/pathname.hpp"

#include "device/time.hpp"
#include "device/keyboard.hpp"
#include "device/mouse.hpp"
#include "device/eventq.hpp"

#include "mathex/transf3d.hpp"
#include "mathex/eulerang.hpp"
#include "mathex/quatern.hpp"
#include "mathex/line3d.hpp"

#include "render/device.hpp"

#include "world4d/Scene/Camera.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Scene/Domain.hpp"

#include "machlog/World/Camera.hpp"
#include "machlog/World/Planet.hpp"

#include "phys/MotionControl/ZenithFlyControl.hpp"
#include "phys/MotionControl/FlyControl.hpp"
#include "phys/MotionControl/ZenithFlyControl.hpp"
#include "phys/MotionControl/GroundFlyControl.hpp"

#include "planeted/planeted.hpp"

#define EXPECT(expected)                                                                                               \
    i >> c;                                                                                                            \
    if (c != expected)                                                                                                 \
        return i;

std::istream& operator>>(std::istream& i, MexPoint3d& p)
{
    char c;

    EXPECT('(');
    i >> p[0];
    EXPECT(',');
    i >> p[1];
    EXPECT(',');
    i >> p[2];
    EXPECT(')');

    return i;
}

std::istream& operator>>(std::istream& i, MexEulerAngles& p)
{
    char c;
    MATHEX_SCALAR a, e, r;

    EXPECT('(');
    i >> a;
    i >> e;
    i >> r;
    EXPECT(')');

    p.azimuth(a);
    p.elevation(e);
    p.roll(r);

    return i;
}

void SDLApp::processInput()
{
    DevEventQueue& devEventQueue = DevEventQueue::instance();

    while (! devEventQueue.isEmpty())
    {
        DevButtonEvent be = DevEventQueue::instance().oldestEvent();

        if (be.isKeyEvent())
        {
            PedPlanetEditor::instance().processInput(be);
            PhysMotionControl::processButtonEvent(be);
            MachCameras::instance().processButtonEvent(be);

            checkForQuit(be);
        }
    }
}

/* End D3DAPP.CPP ***************************************************/
