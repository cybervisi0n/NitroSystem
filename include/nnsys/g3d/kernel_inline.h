#ifndef NNSG3D_KERNEL_INLINE_H_
#define NNSG3D_KERNEL_INLINE_H_

#include <nnsys/g3d/config.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void NNS_G3dAnmObjSetFrame (NNSG3dAnmObj * pAnmObj, fx32 frame)
{
    NNS_G3D_NULL_ASSERT(pAnmObj);
    pAnmObj->frame = frame;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void NNS_G3dAnmObjSetBlendRatio (NNSG3dAnmObj * pAnmObj, fx32 ratio)
{
    NNS_G3D_NULL_ASSERT(pAnmObj);
    pAnmObj->ratio = ratio;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE fx32 NNS_G3dAnmObjGetNumFrame (const NNSG3dAnmObj * pAnmObj)
{
    const NNSG3dResAnmCommon * p;
    NNS_G3D_NULL_ASSERT(pAnmObj);
    NNS_G3D_ASSERT(NNSi_G3dIsValidAnmRes(pAnmObj->resAnm));

    p = (const NNSG3dResAnmCommon *) pAnmObj->resAnm;
    return p->numFrame * FX32_ONE;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void NNS_G3dRenderObjSetFlag (NNSG3dRenderObj * pRenderObj, NNSG3dRenderObjFlag flag)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    pRenderObj->flag |= flag;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void NNS_G3dRenderObjResetFlag (NNSG3dRenderObj * pRenderObj, NNSG3dRenderObjFlag flag)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    pRenderObj->flag &= ~flag;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE BOOL NNS_G3dRenderObjTestFlag (const NNSG3dRenderObj * pRenderObj, NNSG3dRenderObjFlag flag)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    return (pRenderObj->flag & flag) == flag;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE u8 * NNS_G3dRenderObjSetUserSbc (NNSG3dRenderObj * pRenderObj, u8 * sbc)
{
    u8 * rval;
    NNS_G3D_NULL_ASSERT(pRenderObj);

    rval = pRenderObj->ptrUserSbc;
    pRenderObj->ptrUserSbc = sbc;
    return rval;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void NNS_G3dRenderObjSetJntAnmBuffer (NNSG3dRenderObj * pRenderObj, struct NNSG3dJntAnmResult_ * buf)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(buf);

    pRenderObj->recJntAnm = buf;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void NNS_G3dRenderObjSetMatAnmBuffer (NNSG3dRenderObj * pRenderObj, struct NNSG3dMatAnmResult_ * buf)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(buf);

    pRenderObj->recMatAnm = buf;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE struct NNSG3dJntAnmResult_ * NNS_G3dRenderObjReleaseJntAnmBuffer (NNSG3dRenderObj * pRenderObj)                                             {
    struct NNSG3dJntAnmResult_ * rval;
    NNS_G3D_NULL_ASSERT(pRenderObj);

    if (!pRenderObj->recMatAnm)
        pRenderObj->flag &= ~NNS_G3D_RENDEROBJ_FLAG_RECORD;

    rval = pRenderObj->recJntAnm;
    pRenderObj->recJntAnm = NULL;
    return rval;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE struct NNSG3dMatAnmResult_ * NNS_G3dRenderObjReleaseMatAnmBuffer (NNSG3dRenderObj * pRenderObj)                                             {
    struct NNSG3dMatAnmResult_ * rval;
    NNS_G3D_NULL_ASSERT(pRenderObj);

    if (!pRenderObj->recJntAnm)
        pRenderObj->flag &= ~NNS_G3D_RENDEROBJ_FLAG_RECORD;

    rval = pRenderObj->recMatAnm;
    pRenderObj->recMatAnm = NULL;
    return rval;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void * NNS_G3dRenderObjSetUserPtr (NNSG3dRenderObj * pRenderObj, void * ptr)
{
    void * rval = pRenderObj->ptrUser;
    pRenderObj->ptrUser = ptr;
    return rval;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE NNSG3dResMdl * NNS_G3dRenderObjGetResMdl (NNSG3dRenderObj * pRenderObj)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    return pRenderObj->resMdl;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void NNS_G3dRenderObjSetBlendFuncMat (NNSG3dRenderObj * pRenderObj, NNSG3dFuncAnmBlendMat func)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(func);

    pRenderObj->funcBlendMat = func;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void NNS_G3dRenderObjSetBlendFuncJnt (NNSG3dRenderObj * pRenderObj, NNSG3dFuncAnmBlendJnt func)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(func);

    pRenderObj->funcBlendJnt = func;
}

#ifdef SDK_PORT
static
#endif
NNS_G3D_INLINE void NNS_G3dRenderObjSetBlendFuncVis (NNSG3dRenderObj * pRenderObj, NNSG3dFuncAnmBlendVis func)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(func);

    pRenderObj->funcBlendVis = func;
}

#ifdef __cplusplus
}
#endif

#endif
