/*
 * M A N A G E R . I P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of inline non-template methods and inline global functions

#ifdef _INLINE
#define _CODE_INLINE inline
#else
#define _CODE_INLINE
#endif

//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
void W4dManager::time(const PhysAbsoluteTime& newTime)
{
    currentTime_ = newTime;
    generateGeneralPositionId();
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
const PhysAbsoluteTime& W4dManager::time() const
{
    return (hasArtificialTime_ ? artificialTime_ : currentTime_);
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
void W4dManager::sceneManager(W4dSceneManager* pSceneManager)
{
    PRE(pSceneManager != nullptr);

    pSceneManager_ = pSceneManager;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
W4dSceneManager* W4dManager::sceneManager() const
{
    PRE(pSceneManager_ != nullptr);

    return pSceneManager_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
bool W4dManager::hasSceneManager() const
{
    return pSceneManager_ != nullptr;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
uint32_t W4dManager::generateGeneralPositionId()
{
    return ++generalPositionId_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
uint32_t W4dManager::generalPositionId() const
{
    return generalPositionId_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
void W4dManager::artificialTime(const PhysAbsoluteTime& newTime)
{
    hasArtificialTime_ = true;
    artificialTime_ = newTime;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
void W4dManager::clearArtificialTime()
{
    hasArtificialTime_ = false;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
bool W4dManager::hasArtificialTime() const
{
    return hasArtificialTime_;
}
//////////////////////////////////////////////////////////////////////////////////////////
_CODE_INLINE
void W4dManager::clearSceneManager()
{
    pSceneManager_ = nullptr;
}

/* End MANAGER.IPP **************************************************/
