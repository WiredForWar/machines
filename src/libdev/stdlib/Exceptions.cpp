/*
 * E X C E P T I O . C P P
 * (c) Charybdis Limited, 1995. All Rights Reserved.
 */

#include <cstdlib>
#include <cstring>

#include "stdlib/Exceptions.hpp"

//////////////////////////////////////////////////////////////////////

static const char* hs(const char* s)
// copy string to heap
{
    const char* d;
    if (s != nullptr)
    {
        d = (const char*)malloc(1 + strlen(s));
        if (d != nullptr)
            return (const char*)strcpy((char*)d, s);
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////

xmsg::xmsg(const char* what)
    : _What(hs(what))
    , _Alloced(true)
{
    /* Intentionally Empty  */
}

xmsg::xmsg(const char* what, bool copyfl)
    : _What(copyfl ? hs(what) : what)
    , _Alloced(copyfl)
{
    /* Intentionally Empty  */
}

xmsg::xmsg(const xmsg& rhs)
    : _What(hs(rhs.what()))
    , _Alloced(true)
{
    /* Intentionally Empty  */
}

xmsg::~xmsg()
{
    _Tidy();
}

const xmsg& xmsg::operator=(const xmsg& rhs)
{
    if (this == &rhs)
        return *this;

    _Tidy();
    _What = hs(rhs.what());
    _Alloced = true;

    return *this;
}

//////////////////////////////////////////////////////////////////////

const char* xmsg::what() const
{
    return _What != nullptr ? _What : "";
}

void xmsg::_Tidy()
{
    if (_Alloced)
    {
        free((void*)_What);
        _Alloced = false;
    }
}

//////////////////////////////////////////////////////////////////////

logic_error::logic_error(const char* what)
    : xmsg(what)
{
    /* Intentionally Empty  */
}

logic_error::~logic_error()
{
    /*  Intentionally Empty  */
}

//////////////////////////////////////////////////////////////////////

bad_cast::bad_cast(const char* what)
    : logic_error(what)
{
    /* Intentionally Empty  */
}

bad_cast::~bad_cast()
{ /*  Intentionally Empty  */
}

//////////////////////////////////////////////////////////////////////

bad_typeid::bad_typeid(const char* what)
    : logic_error(what)
{
    /* Intentionally Empty  */
}

bad_typeid::~bad_typeid()
{ /*  Intentionally Empty  */
}

//////////////////////////////////////////////////////////////////////

invalid_argument::invalid_argument(const char* what)
    : logic_error(what)
{
    /* Intentionally Empty  */
}

invalid_argument::~invalid_argument()
{
    /*  Intentionally Empty  */
}

//////////////////////////////////////////////////////////////////////

length_error::length_error(const char* what)
    : logic_error(what)
{
    /* Intentionally Empty  */
}

length_error::~length_error()
{ /*  Intentionally Empty  */
}

//////////////////////////////////////////////////////////////////////

out_of_range::out_of_range(const char* what)
    : logic_error(what)
{
    /* Intentionally Empty  */
}

out_of_range::~out_of_range()
{ /*  Intentionally Empty  */
}

//////////////////////////////////////////////////////////////////////

domain_error::domain_error(const char* what)
    : logic_error(what)
{
    /* Intentionally Empty  */
}

domain_error::~domain_error()
{ /*  Intentionally Empty  */
}

//////////////////////////////////////////////////////////////////////

runtime_error::runtime_error(const char* what)
    : xmsg(what)
{
    /* Intentionally Empty  */
}

runtime_error::runtime_error(const char* what, bool copy)
    : xmsg(what, copy)
{
    /* Intentionally Empty  */
}

runtime_error::~runtime_error()
{
    /*  Intentionally Empty */
}

//////////////////////////////////////////////////////////////////////

overflow_error::overflow_error(const char* what)
    : runtime_error(what)
{
    /* Intentionally Empty  */
}

overflow_error::~overflow_error()
{
    /*  Intentionally Empty */
}

//////////////////////////////////////////////////////////////////////

range_error::range_error(const char* what)
    : runtime_error(what)
{
    /* Intentionally Empty  */
}

range_error::~range_error()
{ /*  Intentionally Empty */
}

//////////////////////////////////////////////////////////////////////

bad_alloc::bad_alloc(const char* what)
    : runtime_error(what)
{
    /* Intentionally Empty  */
}

bad_alloc::~bad_alloc()
{ /*  Intentionally Empty */
}

//////////////////////////////////////////////////////////////////////

static fvoid_t* terminate_handler = &abort;

fvoid_t* set_terminate(fvoid_t* newh)
{
    fvoid_t* oldh = terminate_handler;
    terminate_handler = newh;
    return oldh;
}

void terminate()
// call terminate handler or abort
{
    if (terminate_handler != nullptr)
        (*terminate_handler)();

    abort();
}

//////////////////////////////////////////////////////////////////////

static fvoid_t* unexpected_handler = &terminate;

fvoid_t* set_unexpected(fvoid_t* newh)
{
    fvoid_t* oldh = unexpected_handler;
    unexpected_handler = newh;
    return oldh;
}

void unexpected()
// call unexpected handler or terminate
{
    if (unexpected_handler != nullptr)
        (*unexpected_handler)();

    terminate();
}

/* End EXCEPTIO.CPP *************************************************/
