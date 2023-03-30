/*
 * A N I M A T I O . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _MACH_ANIMATIO_HPP
#define _MACH_ANIMATIO_HPP

#include "mathex/radians.hpp"

#include "machphys/machphys.hpp"

#include "machlog/operatio.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachLogAnimation : public MachLogOperation
// cannonical form revoked
{
public:
    MachLogAnimation();

    ~MachLogAnimation() override;

    ///////////////////////////////

    PER_MEMBER_PERSISTENT_ABSTRACT(MachLogAnimation);
    PER_FRIEND_READ_WRITE(MachLogAnimation);

protected:
    virtual PhysRelativeTime doStartAnimation() = 0;

    bool doStart() override;

    // redefinition optional, invocation optional
    PhysRelativeTime doUpdate() override;

    void doFinish() override;

    // inherited from MachIrreducibleOperation
    // redefinition optional, invocation optional
    bool doBeInterrupted() override;

    bool doIsFinished() const override;

    void doOutputOperator(std::ostream&) const override = 0;

private:
    MachLogAnimation(const MachLogAnimation&);
    MachLogAnimation& operator=(const MachLogAnimation&);
    bool operator==(const MachLogAnimation&) const;

    PhysAbsoluteTime endTime_;
};

PER_DECLARE_PERSISTENT(MachLogAnimation);

/* //////////////////////////////////////////////////////////////// */

// forward refs
class MachLogCanTurn;

class MachLogTurnAnimation : public MachLogAnimation
// cannonical form revoked
{
public:
    MachLogTurnAnimation(MachLogCanTurn* pActor, MexRadians angle);

    ~MachLogTurnAnimation() override;

    ///////////////////////////////

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogTurnAnimation);
    PER_FRIEND_READ_WRITE(MachLogTurnAnimation);

protected:
    PhysRelativeTime doStartAnimation() override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogTurnAnimation(const MachLogTurnAnimation&);
    MachLogTurnAnimation& operator=(const MachLogTurnAnimation&);
    bool operator==(const MachLogTurnAnimation&) const;

    MachLogCanTurn* pActor_;
    MexRadians turnBy_;
};

PER_DECLARE_PERSISTENT(MachLogTurnAnimation);

/* //////////////////////////////////////////////////////////////// */

/*
class MachLogConstructor;
class MachLogConstruction;

class MachLogConstructAnimation
: public MachLogAnimation
// cannonical form revoked
{
public:

    MachLogConstructAnimation( MachLogConstructor * pActor,
                            MachLogConstruction *pConstr,
                            MachPhys::BuildingMaterialUnits units );

    virtual ~MachLogConstructAnimation();

    ///////////////////////////////

    PER_MEMBER_PERSISTENT_VIRTUAL( MachLogConstructAnimation );
    PER_FRIEND_READ_WRITE( MachLogConstructAnimation );

protected:

    virtual PhysRelativeTime doStartAnimation();

    virtual PhysRelativeTime doUpdate( );

    virtual void doOutputOperator( ostream& ) const;

private:

    MachLogConstructAnimation( const MachLogConstructAnimation& );
    MachLogConstructAnimation& operator =( const MachLogConstructAnimation& );
    bool operator ==( const MachLogConstructAnimation& ) const;

    MachLogConstructor *                        pActor_;
    MachLogConstruction *                       pConstruction_;
    MachPhys::BuildingMaterialUnits             units_;
    PhysAbsoluteTime                            lastUpdateTime_;
};

PER_DECLARE_PERSISTENT( MachLogConstructAnimation );
*/

/* //////////////////////////////////////////////////////////////// */

// forward refs
class MachActor;

class MachLogBeDestroyedAnimation : public MachLogAnimation
// cannonical form revoked
{
public:
    MachLogBeDestroyedAnimation(MachActor* pActor);

    ~MachLogBeDestroyedAnimation() override;

    ///////////////////////////////

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogBeDestroyedAnimation);
    PER_FRIEND_READ_WRITE(MachLogBeDestroyedAnimation);

protected:
    PhysRelativeTime doStartAnimation() override;
    void doFinish() override;

    void doOutputOperator(std::ostream&) const override;

private:
    MachLogBeDestroyedAnimation(const MachLogBeDestroyedAnimation&);
    MachLogBeDestroyedAnimation& operator=(const MachLogBeDestroyedAnimation&);
    bool operator==(const MachLogBeDestroyedAnimation&) const;

    MachActor* pActor_;
};

PER_DECLARE_PERSISTENT(MachLogBeDestroyedAnimation);

/* //////////////////////////////////////////////////////////////// */

#endif /*  #ifndef _MACH_ANIMATIO_HPP  */

/* End ANIMATIO.HPP *************************************************/