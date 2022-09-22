/*
 * D I A G . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef BASE_DiagStreamType_HPP
#define BASE_DiagStreamType_HPP

enum DiagStreamType
{
    DIAG_BOB,
    DIAG_HAL,
    DIAG_IAIN,
    DIAG_JERRY,
    DIAG_RICHARD,
    DIAG_JON,
    DIAG_YUEAI,
    DIAG_LIONEL,
    DIAG_CERI,
    DIAG_NEIL,

    DIAG_MISC,
    DIAG_WHERE,
    DIAG_TRACE,
    DIAG_MEMORY,
    DIAG_RENDER,

    DIAG_PER_READ,
    DIAG_PER_WRITE,

    DIAG_NETWORK,

    DIAG_WAYNE,

    DIAG_DANIEL,
    DIAG_MOTSEQ, // MachLogMotionSequencer
    DIAG_CS2PATH, // Config space pathfinding stuff (domain and normal)
    DIAG_CS2VGRA, // Visibility graph diagnostics
    DIAG_OBSERVER, // trapping observer pattern problems - stream added to due volume of output

    DIAG_PATH_PROFILE, // Debugging the path profile across the planet surface

    DIAG_SOUND,

    DIAG_REC_RECORD,
    DIAG_REC_PLAYBACK,

    DIAG_NET_ANALYSIS,

    DIAG_A_STAR,

    DIAG_LINEAR_TRAVEL, // Stream for use by PhysLinearTravelPlan

    DIAG_PLANET_SURFACE, // Used for reporting errors in a planet surface
    DIAG_PHYS_MOTION, // Physical motion stream

    DIAG_NETWORK_ERRORS, // Specifically to help us spot network errors (such as loss of synchronicity)

    DIAG_MEX_GRID2D,

    DIAG_CONFIG_SPACE,

    //  Extra streams should be added immediately before this line

    DIAG_N_DIAG_STREAMS //  Do not use this value in any code. It is liable to change without warning or recompiles
};

#endif // BASE_DiagStreamType_HPP
