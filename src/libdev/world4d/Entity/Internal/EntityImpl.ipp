#ifdef _INLINE
#define _CODE_INLINE inline
#else
#define _CODE_INLINE
#endif

_CODE_INLINE
bool W4dEntityImpl::hasAnimationData() const
{
    return pAnimationDataPtrs_ != nullptr || pAnimationLightDataPtrs_ != nullptr;
}

_CODE_INLINE
bool W4dEntityImpl::hasMesh() const
{
    return hasMesh_;
}

_CODE_INLINE
void W4dEntityImpl::passId(uint32_t id)
{
    passId_ = id;
}

_CODE_INLINE
uint32_t W4dEntityImpl::passId() const
{
    return passId_;
}

_CODE_INLINE
bool W4dEntityImpl::hasParent() const
{
    return pParent_ != nullptr;
}

_CODE_INLINE
W4dEntity* W4dEntityImpl::pParent() const
{

    PRE(hasParent());

    W4dEntity* result = pParent_;

    POST(result != nullptr);

    return result;
}

_CODE_INLINE
bool W4dEntityImpl::hasLightList() const
{
    return pLocalLightList_ != nullptr;
}

_CODE_INLINE
void W4dEntityImpl::lightList(W4dLocalLightList* pList)
{
    pLocalLightList_ = pList;
}

_CODE_INLINE
const W4dLocalLightList& W4dEntityImpl::lightList() const
{
    PRE(hasLightList());
    return *pLocalLightList_;
}

_CODE_INLINE
void W4dEntityImpl::isComposite(bool isIt)
{
    isComposite_ = isIt;
}

_CODE_INLINE
bool W4dEntityImpl::isComposite() const
{
    return isComposite_;
}

_CODE_INLINE
void W4dEntityImpl::doNotLight(bool lightIt)
{
    doNotLight_ = lightIt;
}

_CODE_INLINE
bool W4dEntityImpl::doNotLight() const
{
    return doNotLight_;
}

_CODE_INLINE
void W4dEntityImpl::clientData(int newValue)
{
    clientData_ = newValue;
}

_CODE_INLINE
int W4dEntityImpl::clientData() const
{
    return clientData_;
}
