/*
 * E N T S C A L E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "world4d/entscale.hpp"

#include "render/scale.hpp"

#ifndef _INLINE
#include "world4d/entscale.ipp"
#endif

PER_DEFINE_PERSISTENT(W4dEntityScale);

W4dEntityScale::W4dEntityScale()
    : pUnscaledTransformPosition_(nullptr)
    , pScale_(nullptr)
{

    TEST_INVARIANT;
}

W4dEntityScale::~W4dEntityScale()
{
    TEST_INVARIANT;

    delete pUnscaledTransformPosition_;
    delete pScale_;
}

void W4dEntityScale::meshScale(const RenScale& scale)
{
    PRE(! scale.isUnity());

    // If we have an existing scale of the same type, use assignment
    if (pScale_)
    {
        bool existingUniform = pScale_->isUniform();
        bool newUniform = scale.isUniform();
        if (existingUniform && newUniform)
            pScale_->asUniform() = scale.asUniform();
        else if (! existingUniform && ! newUniform)
            pScale_->asNonUniform() = scale.asNonUniform();
        else
        {
            delete pScale_;
            pScale_ = nullptr;
        }
    }

    // If still don't have a scale, copy the argument
    if (! pScale_)
        pScale_ = scale.clone();
}

void W4dEntityScale::meshScale(const RenUniformScale& scale)
{
    // If we have an existing scale of the same type, use assignment
    if (pScale_)
    {
        bool existingUniform = pScale_->isUniform();
        if (existingUniform)
            pScale_->asUniform() = scale;
        else
        {
            delete pScale_;
            pScale_ = nullptr;
        }
    }

    // If still don't have a scale, copy the argument
    if (! pScale_)
        pScale_ = scale.clone();
}

void W4dEntityScale::meshScale(const RenNonUniformScale& scale)
{
    // If we have an existing scale of the same type, use assignment
    if (pScale_)
    {
        bool existingNonUniform = pScale_->isNonUniform();
        if (existingNonUniform)
            pScale_->asNonUniform() = scale;
        else
        {
            delete pScale_;
            pScale_ = nullptr;
        }
    }

    // If still don't have a scale, copy the argument
    if (! pScale_)
        pScale_ = scale.clone();
}

void W4dEntityScale::scaleTransformFromParent(bool doScale)
{
    if (doScale)
    {
        // Ensure we have the 3d point allocated to store the unscaled position
        if (pUnscaledTransformPosition_ == nullptr)
            pUnscaledTransformPosition_ = new MexPoint3d;
    }
    else
    {
        // Clear the 3d point
        delete pUnscaledTransformPosition_;
        pUnscaledTransformPosition_ = nullptr;
    }
}

void W4dEntityScale::unscaledTransformPosition(const MexPoint3d& p)
{
    PRE(scaleTransformFromParent());

    *pUnscaledTransformPosition_ = p;
}

void W4dEntityScale::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const W4dEntityScale& t)
{

    o << "W4dEntityScale " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "W4dEntityScale " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const W4dEntityScale& scale)
{
    ostr << scale.pUnscaledTransformPosition_;
    ostr << scale.pScale_;
}

void perRead(PerIstream& istr, W4dEntityScale& scale)
{
    istr >> scale.pUnscaledTransformPosition_;
    istr >> scale.pScale_;
}

/* End ENTSCALE.CPP *************************************************/
