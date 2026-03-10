/*
 * E X C E P T I O . H P P
 * (c) Charybdis Limited, 1995. All Rights Reserved.
 */

#include "base/base.hpp"

#ifndef _STD_EXCEPTIO_HPP
#define _STD_EXCEPTIO_HPP

#ifndef NO_EXCEPTIONS

// exceptions implemented

#define _TRY_BEGIN                                                                                                     \
    try                                                                                                                \
    {
#define _CATCH_ALL                                                                                                     \
    catch (...)                                                                                                        \
    {
#define _CATCH_END }
#define _RAISE(x) throw(x)
#define _RERAISE(x) throw

#else

// exceptions revoked

#define _TRY_BEGIN {
#define _CATCH_ALL {
#define _CATCH_END }
#define _RAISE(x)
#define _RERAISE(x)

#endif /* #ifndef NO_EXCEPTIONS */

//////////////////////////////////////////////////////////////////////

using fvoid_t = void();

void terminate();
fvoid_t* set_terminate(fvoid_t*);

void unexpected();
fvoid_t* set_unexpected(fvoid_t*);

//////////////////////////////////////////////////////////////////////

// xmsg is the base class
// from which all specialised exceptions are derived

class xmsg
{
public:
    xmsg(const char* what = nullptr);

    xmsg(const xmsg& rhs);

    virtual ~xmsg();

    const xmsg& operator=(const xmsg& rhs);

    ///////////////////////////////

    virtual const char* what() const;

    // handle an unthrowable xmsg
    static void _Throw(xmsg* ePtr);

    using raise_handler = void (*)(xmsg&);
    static raise_handler set_raise_handler(raise_handler newh);

protected:
    xmsg(const char*, bool);

    virtual void do_raise();

private:
    void _Raise();
    void _Tidy();

    static raise_handler _Handler;

    const char* _What;
    bool _Alloced;
};

//////////////////////////////////////////////////////////////////////

// xmsgs derived from logic_error represents errors which
// can, in principle, be detected and avoided at\before compile-time

class logic_error : public xmsg
{
public:
    logic_error(const char* what = nullptr);

    ~logic_error() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// domain_error is for reporting precondition violations

class domain_error : public logic_error
{
public:
    domain_error(const char* what = nullptr);

    ~domain_error() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// invalid_argument is the base class for reporting an invalid argument

class invalid_argument : public logic_error
{
public:
    invalid_argument(const char* what = nullptr);

    ~invalid_argument() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// length_error is the base class for reporting an attempt to produce
// an object whose length equals or exceeds NPOS.

class length_error : public logic_error
{
public:
    length_error(const char* what = nullptr);

    ~length_error() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// out_of_range is the base class for reporting out-of-range arguments

class out_of_range : public logic_error
{
public:
    out_of_range(const char* what = nullptr);

    ~out_of_range() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// bad_cast indicates an the execution of an invalid dynamic-cast

class bad_cast : public logic_error
{
public:
    bad_cast(const char* what = nullptr);

    ~bad_cast() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// bad_typeid indicates the invocation of typeid( *p )
// for a null pointer p.

class bad_typeid : public logic_error
{
public:
    bad_typeid(const char* what = nullptr);

    ~bad_typeid() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// xmsgs derived from runtime_error represents errors which
// cannot be detected at compile-time, such as invalid user input
// or running out of heap space

class runtime_error : public xmsg
{
public:
    runtime_error(const char* what = nullptr);

    ~runtime_error() override;

protected:
    runtime_error(const char* what, bool copy);

    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// range_error is for reporting postcondition violations

class range_error : public runtime_error
{
public:
    range_error(const char* what = nullptr);

    ~range_error() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// overflow_error is the base class for xmsgs thrown by the
// standard c++ library to indicate an arithmetic overflow

class overflow_error : public runtime_error
{
public:
    overflow_error(const char* what = nullptr);

    ~overflow_error() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

// bad_alloc is the base class for xmsgs
// thrown to report a failure to allocate storage
// errno : ENOMEM

class bad_alloc : public runtime_error
{
public:
    bad_alloc(const char* what = nullptr);

    ~bad_alloc() override;

protected:
    void do_raise() override;
};

//////////////////////////////////////////////////////////////////////

#endif /* #ifndef _STD_EXCEPTIO_HPP    */
