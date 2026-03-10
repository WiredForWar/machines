/*
 * T I . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "ctl/Vector.hpp"
#include "ctl/Vector.ctp"
#include "ctl/List.hpp"
#include "ctl/List.ctp"

class MotionControl;
class ModelData;

void TerrainDemoDummyFunction1()
{
    static ctl_list<MotionControl*> motionList;
    static ctl_vector<ModelData*> dummyModels;
}

/* End TI.CPP *****************************************************/
