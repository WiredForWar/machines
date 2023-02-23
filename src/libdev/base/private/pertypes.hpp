/*
 * P E R T Y P E S . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    Internal typedefs used by the persistence code
*/

#ifndef _PER_INTERNAL_PERTYPES_HPP
#define _PER_INTERNAL_PERTYPES_HPP

using PerIdentifier = uint32;

//  There seems to be some problem with storing pointers in a map so we use a
//  type that should allow error free casting in both directions.

using PerMapPtrType = size_t;

class PerIstream;
class PerOstream;

using PerReadFnPtr = void (*)(PerIstream&);
using PerWriteFnPtr = void (*)(PerOstream&, const void*);

#endif

/* End PERSIST.HPP ******************************************************/
