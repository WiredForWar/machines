/*
 * P N D P L A N . C T P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline template methods

template <class X> PLAN_TYPE<X>::PLAN_TYPE()
{

    TEST_INVARIANT;
}

template <class X> PLAN_TYPE<X>::~PLAN_TYPE()
{
    TEST_INVARIANT;
}

template <class X> void PLAN_TYPE<X>::CLASS_INVARIANT
{
}

std::ostream& operator<<(std::ostream& o, const PLAN_TYPE<X>& t)
{

    o << "PLAN_TYPE< X > " << static_cast<const void*>(&t) << " start" << endl;
    o << "PLAN_TYPE< X > " << static_cast<const void*>(&t) << " end" << endl;

    return o;
}

/* End PNDPLAN.CTP **************************************************/
