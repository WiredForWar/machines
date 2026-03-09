/*
 * T I 1 . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

// #define _INSTANTIATE_TEMPLATE_CLASSES
//  #define _INSTANTIATE_TEMPLATE_FUNCTIONS

#include "ctl/list.hpp"
#include "ctl/list.ctp"
#include "ctl/vector.hpp"
#include "ctl/vector.ctp"

#include "mathex/point2d.hpp"

#include "machlog/Actors/Artefact.hpp"
#include "machlog/World/BuildPoint.hpp"
#include "machlog/Internal/ArtefactsData.hpp"
#include "machlog/Internal/SequencerData.hpp"
////#include "machlog/consdata.hpp"

#include "phys/Plans/MotionChunk.hpp"
#include "machphys/Machines/MachineMoveInfo.hpp"

class MachOperation;
class MachLogMachine;

void ti1()
{
    static ctl_list<MexPoint2d> dummyPath;

    static ctl_vector<MachLogMachine*> dummyMachines;
    static ctl_vector<MachLogArtefactSubType> dummySubTypes;
    static ctl_vector<MachLogArtefactLinkData> dummyLinkDatas;
    static ctl_vector<MachLogBuildPoint> dummyBuildPoints;
    //    static MachLogConstructionData::Entrances entrances;

    static ctl_vector<PhysMotionChunk> dummy1_;
    static ctl_vector<MachPhysMachineMoveInfo> dummy2_;
}

/* End TI.CPP *******************************************************/
