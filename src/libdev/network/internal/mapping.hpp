#ifndef _MAPPING_HPP
#define	_MAPPING_HPP
/*******************************************************************
 DESCRIPTION: This class contains the static mapping functions
 required to map between Windows specific types (GUID_t and DPID_t), and
 their non-Windows equivalents.
 *******************************************************************/


//#include "dplay.h"

#include "ctl/vector.hpp"

using DPID_t = int;
using GUID_t = int;

class NetMappings
{
public:
	static unsigned mapDPIDtoUnsigned( DPID_t );
	static DPID_t mapUnsignedtoDPID( unsigned mapUnsigned );
	//PRE(mapUnsigned < currentDPIDS().size());

	//Update the given map index with the new DPID_t
	static void updateDPID( unsigned mapUnsigned, DPID_t newMapID );
	//PRE(mapUnsigned < currentDPIDS().size());

	static unsigned mapGUIDtoUnsigned( GUID_t );
	static GUID_t mapUnsignedtoGUID( unsigned mapUnsigned );
	//PRE(mapUnsigned < currentGUIDS().size());

private:
	typedef ctl_vector<GUID_t> GUIDVector;
	static GUIDVector& currentGUIDS();

	typedef ctl_vector<DPID_t> DPIDVector;
	static DPIDVector& currentDPIDS();
};

#endif //_MAPPING_HPP
