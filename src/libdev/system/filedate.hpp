/*
 * F I L E D A T E . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved
 */

/*
    SysFileDate

    Stores file date information.
*/

#ifndef _SYS_FILEDATE_HPP
#define _SYS_FILEDATE_HPP

#include "base/base.hpp"

class SysFileDate
{
public:
    SysFileDate();
    //  POST( !dateSet() );

    SysFileDate(size_t year, size_t month, size_t dayOfMonth);
    //  POST( dateSet() );

    SysFileDate(const SysFileDate& copyMe);
    //  POST( dateSet() );

    SysFileDate& operator=(const SysFileDate&);

    ~SysFileDate();

    bool dateSet() const;

    bool yearSet() const;
    size_t year() const;
    //  PRE( yearSet() );

    bool monthSet() const;
    size_t month() const;
    //  PRE( monthSet() );
    //  POST( result >= 1 && result <= 12 )

    bool dayOfMonthSet() const;
    size_t dayOfMonth() const;
    //  PRE( dayOfMonthSet() );
    //  POST( result >= 1 && result <= 31 )

    void CLASS_INVARIANT { INVARIANT(this != nullptr); }

private:
    // Operation deliberately revoked
    bool operator==(const SysFileDate&);

    size_t year_;
    size_t month_;
    size_t dayOfMonth_;

    bool yearSet_;
    bool monthSet_;
    bool dayOfMonthSet_;
    bool dateSet_;
};

std::ostream& operator<<(std::ostream&, const SysFileDate&);

#endif

/* End SysFileDate.HPP *************************************************/
