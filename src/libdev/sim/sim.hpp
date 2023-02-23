/*
 * S I M . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    General include for the Sim library, with forward declarations.
*/

#ifndef _SIM_HPP
#define _SIM_HPP

// Forward declarations
using SimPriority = uint;
class SimManager;
class SimProcess;
class SimActor;
class SimEventDiary;
class SimDiscreteEvent;

// Collection of SimActor*'s
template <class T> class ctl_vector;
using SimActors = ctl_vector<SimActor*>;

#endif // ndefined _SIM_HPP
/* End SIM.HPP **************************************************/
