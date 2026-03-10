
/***********************************************************

  S K Y . I P P
  (c) Charybdis Limited, 1998. All Rights Reserved

***********************************************************/

#ifdef _INLINE
#define _CODE_INLINE inline
#else
#define _CODE_INLINE
#endif

_CODE_INLINE
void EnvSky::attachStars(W4dStars* const pStars)
{
    PRE(pStars);

    pStars_ = pStars;
}

_CODE_INLINE
void EnvSky::detachStars()
{
    PRE(pStars_);

    pStars_ = nullptr;
}

_CODE_INLINE
const W4dStars* EnvSky::pStars() const
{
    return pStars_;
}

_CODE_INLINE
W4dStars* EnvSky::pStars()
{
    return pStars_;
}

_CODE_INLINE
const RenColour& EnvSky::overrideColour() const
{
    return overrideColour_;
}

_CODE_INLINE
bool EnvSky::doColourOverride() const
{
    return doColourOverride_;
}

_CODE_INLINE
bool EnvSky::colourChanged() const
{
    return colourChanged_;
}

_CODE_INLINE
void EnvSky::colourChangeUpdated()
{
    colourChanged_ = false;
    POST(!colourChanged());
}

////////////////////////////////////////////////////////////

_CODE_INLINE
void EnvControlledSky::controller(const EnvSatellite* pController)
{
    PRE(pController);

    pController_ = pController;
}

_CODE_INLINE
const EnvSatellite* EnvControlledSky::controller() const
{
    return pController_;
}

////////////////////////////////////////////////////////////

// VIRTUAL //
_CODE_INLINE
bool EnvUniformSky::visible() const
{
    return visible_;
}

// VIRTUAL //
_CODE_INLINE
void EnvUniformSky::visible(bool yesNo)
{
    visible_ = yesNo;
}

_CODE_INLINE
void EnvUniformSky::colourTable(EnvElevationColourTable* pColourTable)
{
    PRE(pColourTable);

    pColourTable_ = pColourTable;
}

////////////////////////////////////////////////////////////

_CODE_INLINE
void EnvStaticSky::backgroundColour(RenColour newBackgroundColour)
{
    backgroundColour_ = newBackgroundColour;
}

_CODE_INLINE
const RenColour& EnvStaticSky::backgroundColour() const
{
    return backgroundColour_;
}

/* End SKY.IPP ********************************************/
