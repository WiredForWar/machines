/*
 * E R A S E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved.
 */

// Helper functions for safely erasing objects from standard containers.

#ifndef _CTL_ERASE_HPP
#define _CTL_ERASE_HPP

// #include "ctl/algorith.hpp"
#include <algorithm>
#include "ctl/function.hpp"

//////////////////////////////////////////////////////////////////////

#include "ctl/vector.hpp"
#include "ctl/list.hpp"

//////////////////////////////////////////////////////////////////////

template <class T> bool ctl_erase_first_instance(ctl_vector<T>* pC, const T& key)
// returns true iff an instance of key was erased
{
    PRE(pC != nullptr);
    POST_DATA(typename ctl_vector<T>::size_type old_size = pC->size());
    POST_DATA(typename ctl_vector<T>::size_type old_nvalues = ctl_count(*pC, key));

    typename ctl_vector<T>::iterator i = find(pC->begin(), pC->end(), key);
    bool result = i != pC->end();
    if (result)
        pC->erase(i);

    POST(implies(result, pC->size() == old_size - 1));
    POST(implies(result, ctl_count(*pC, key) == old_nvalues - 1));
    POST(implies(!result, pC->size() == old_size));
    return result;
}

///////////////////////////////////

template <class T> bool ctl_erase_first_instance(ctl_list<T>* pC, const T& key)
// returns true iff an instance of key was erased
{
    PRE(pC != nullptr);
    POST_DATA(typename ctl_list<T>::size_type old_size = pC->size());
    POST_DATA(typename ctl_list<T>::size_type old_nvalues = ctl_count(*pC, key));

    typename ctl_list<T>::iterator i = find(pC->begin(), pC->end(), key);
    bool result = i != pC->end();
    if (result)
        pC->erase(i);

    POST(implies(result, pC->size() == old_size - 1));
    POST(implies(result, ctl_count(*pC, key) == old_nvalues - 1));
    POST(implies(!result, pC->size() == old_size));
    return result;
}

//////////////////////////////////////////////////////////////////////

#endif // #ifndef _CTL_ERASE_HPP

/* End ERASE.HPP ****************************************************/
