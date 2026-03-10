/*
 * T I . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "ctl/Vector.hpp"
#include "ctl/Vector.ctp"

class MachPhysComplexityBooleanItem;
class MachPhysComplexityChoiceItem;

void MachDummyFunctionComp()
{
    static ctl_vector<MachPhysComplexityChoiceItem*> dummyVectorComplexityChoiceItem;
    static ctl_vector<MachPhysComplexityBooleanItem*> dummyVectorComplexityBooleanItem;
}

/* End TI.CPP *****************************************************/
