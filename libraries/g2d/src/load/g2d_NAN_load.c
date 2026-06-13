#include <nitro.h>

#include <nnsys/g2d/load/g2d_NAN_load.h>
#include <nnsys/g2d/g2d_Load.h>

#include "g2di_Debug.h"

#ifndef SDK_FINALROM
static const char * s_animTypeStrTbl[] = {
    "NNS_G2D_ANIMATIONTYPE_INVALID",
    "NNS_G2D_ANIMATIONTYPE_CELL",
    "NNS_G2D_ANIMATIONTYPE_MULTICELLLOCATION",
    "NNS_G2D_ANIMATIONTYPE_MAX",
};

static const char * s_elemTypeStrTbl[] = {
    "NNS_G2D_ANIMELEMENT_INDEX",
    "NNS_G2D_ANIMELEMENT_INDEX_SRT",
    "NNS_G2D_ANIMELEMENT_INDEX_T",
    "NNS_G2D_ANIMELEMENT_MAX"
};

static const char * s_playModeStrTbl[] = {
    "NNS_G2D_ANIMATIONPLAYMODE_INVALID",
    "NNS_G2D_ANIMATIONPLAYMODE_FORWARD",
    "NNS_G2D_ANIMATIONPLAYMODE_FORWARD_LOOP",
    "NNS_G2D_ANIMATIONPLAYMODE_REVERSE",
    "NNS_G2D_ANIMATIONPLAYMODE_REVERSE_LOOP",
    "NNS_G2D_ANIMATIONPLAYMODE_MAX"
};
#endif

#ifdef SDK_PORT
#define NAN_DATA_OUT_MAX 64
#define NAN_SEQUENCE_OUT_MAX 400
#define NAN_MAX_FRAMES_OUT 1000
#define NAN_UAAT_DATA_OUT_MAX 200
#define NAN_USER_EX_ANIM_SEQ_ATTR_MAX 400
static u32 curDataOutNum = 0;
static u32 curSequenceOutNum = 0;
static NNSG2dAnimBankData dataOuts[NAN_DATA_OUT_MAX] = {0};
static NNSG2dAnimSequenceData sequenceOuts[NAN_DATA_OUT_MAX][NAN_SEQUENCE_OUT_MAX] = {0};
static u8 frameArrOut[NAN_DATA_OUT_MAX][sizeof(NNSG2dAnimFrameData) * NAN_MAX_FRAMES_OUT];
static u32 curUaatDataOutNum = 0;
static WIN_UAATData uaatDataOuts[NAN_UAAT_DATA_OUT_MAX];
static u8 userExAnimSeqAttrOuts[NAN_UAAT_DATA_OUT_MAX][sizeof(NNSG2dUserExAnimSequenceAttr)*NAN_USER_EX_ANIM_SEQ_ATTR_MAX];

#define NAN_ALLOCATED_ANIM_BANKS_MAX 2048
typedef struct {
    void * origAddr;
    void * mallocAddr;
} WIN_NAN_malloc_t;
static WIN_NAN_malloc_t s_allocatedAnimBanksTbl[NAN_ALLOCATED_ANIM_BANKS_MAX] = {0};

static void WIN_FreeAnimBank(NNSG2dAnimBankData * pAnimBank);
static void WIN_RegisterAnimBank(NNSG2dAnimBankData ** ppAnimBank );

static void WIN_FreeAnimBank(NNSG2dAnimBankData * pAnimBank)
{
    free(pAnimBank->pSequenceArrayHead);
    free(pAnimBank->pFrameArrayHead);
    free(pAnimBank);
    return;
}

static void WIN_RegisterAnimBank(NNSG2dAnimBankData ** ppAnimBank )
{
    for(int i=0; i < NAN_ALLOCATED_ANIM_BANKS_MAX; i++)
    {
        if(s_allocatedAnimBanksTbl[i].origAddr == NULL)
        {
            s_allocatedAnimBanksTbl[i].origAddr = ppAnimBank;
            s_allocatedAnimBanksTbl[i].mallocAddr = *ppAnimBank;
            break;
        }
    }
}

void WIN_CheckAndFreeAnimBank(void * start, void * end)
{
    u64 start64 = (u64)start;
    u64 end64 = (u64)end;
    for(int i=0; i < NAN_ALLOCATED_ANIM_BANKS_MAX; i++)
    {
        if((u64)s_allocatedAnimBanksTbl[i].origAddr > start64 && (u64)s_allocatedAnimBanksTbl[i].origAddr < end64)
        {
            NNSG2dAnimBankData * bank = s_allocatedAnimBanksTbl[i].mallocAddr;
            WIN_FreeAnimBank(bank);
            s_allocatedAnimBanksTbl[i].origAddr = NULL;
            s_allocatedAnimBanksTbl[i].mallocAddr = NULL;
            break;
        }
    }
}
#endif

static BOOL GetUnpackedAnimBankImpl_ (void * pNanrFile, NNSG2dAnimBankData ** ppAnimBank)
{
    NNS_G2D_NULL_ASSERT(pNanrFile);
    NNS_G2D_NULL_ASSERT(ppAnimBank);
    #ifdef SDK_PORT
    {
        NNSG2dBinaryFileHeader*     pBinFile    = (NNSG2dBinaryFileHeader*)pNanrFile;
        {
            NNSG2dAnimBankDataBlock* pAnimBankBlk   =
                (NNSG2dAnimBankDataBlock*)NNS_G2dFindBinaryBlock( pBinFile, 
                                                                  NNS_G2D_BLKSIG_ANIMBANK );
            if( pAnimBankBlk )
            {
                *ppAnimBank = NNS_G2dUnpackNAN( (void*)&pAnimBankBlk->animBankData );

                WIN_RegisterAnimBank(ppAnimBank);
                   
                //
                // OK 
                //
                //*ppAnimBank = &pAnimBankBlk->animBankData;
                return TRUE;   
            }
        }                
    }
    #else
    {
        NNSG2dBinaryFileHeader * pBinFile = (NNSG2dBinaryFileHeader *)pNanrFile;
        {
            NNSG2dAnimBankDataBlock * pAnimBankBlk =
                (NNSG2dAnimBankDataBlock *)NNS_G2dFindBinaryBlock(pBinFile,
                                                                  NNS_G2D_BLKSIG_ANIMBANK);
            if (pAnimBankBlk) {
                NNS_G2dUnpackNAN((void *)&pAnimBankBlk->animBankData);
                *ppAnimBank = &pAnimBankBlk->animBankData;
                return TRUE;
            }
        }
    }
    #endif

    *ppAnimBank = NULL;
    return FALSE;
}

static BOOL CheckAnimSequenceValidity_ (const NNSG2dAnimSequenceData * pSeq)
{
    int i;
    BOOL bHasEffectiveFrame = FALSE;

    for (i = 0; i < pSeq->numFrames; i++) {
        if (pSeq->pAnmFrameArray[i].frames != 0) {
            bHasEffectiveFrame = TRUE;
        }
    }

    return bHasEffectiveFrame;
}

BOOL NNS_G2dGetUnpackedAnimBank (void * pNanrFile, NNSG2dAnimBankData ** ppAnimBank)
{
    NNS_G2D_NULL_ASSERT(pNanrFile);
    NNS_G2D_NULL_ASSERT(ppAnimBank);

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileSignatureValid(pNanrFile,
                                                      NNS_G2D_BINFILE_SIG_CELLANIM),
                      "Input file signature is invalid for this method.");

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileVersionValid(pNanrFile,
                                                    BIN_FILE_VERSION(NANR)),
                      "Input file is obsolete. Please use the new g2dcvtr.exe.");

    return GetUnpackedAnimBankImpl_(pNanrFile, ppAnimBank);
}

BOOL NNS_G2dGetUnpackedMCAnimBank (void * pNanrFile, NNSG2dAnimBankData ** ppAnimBank)
{
    NNS_G2D_NULL_ASSERT(pNanrFile);
    NNS_G2D_NULL_ASSERT(ppAnimBank);

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileSignatureValid(pNanrFile,
                                                      NNS_G2D_BINFILE_SIG_MULTICELLANIM),
                      "Input file signature is invalid for this method.");

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileVersionValid(pNanrFile,
                                                    BIN_FILE_VERSION(NANR)),
                      "Input file is obsolete. Please use the new g2dcvtr.exe.");

    return GetUnpackedAnimBankImpl_(pNanrFile, ppAnimBank);
}

#ifdef SDK_PORT
NNSG2dAnimBankData* NNS_G2dUnpackNAN(NNSG2dAnimBankData* pData)
#else
void NNS_G2dUnpackNAN (NNSG2dAnimBankData * pData)
#endif
{
    u16 i, j;

    NNS_G2D_NULL_ASSERT(pData);
    #ifdef SDK_PORT
    WIN_NNSG2dAnimBankData * pDataWin = (WIN_NNSG2dAnimBankData *)pData;
    NNSG2dAnimBankData * pDataOut = (NNSG2dAnimBankData *)malloc( sizeof(NNSG2dAnimBankData) );

    pDataOut->pAnimContents      = (void *)((u64)pDataWin->pAnimContents + (u64)pDataWin);

    pDataOut->numSequences = pDataWin->numSequences;
    pDataOut->numTotalFrames =  pDataWin->numTotalFrames;

    pDataOut->pSequenceArrayHead = malloc(sizeof( NNSG2dAnimSequenceData ) * pDataOut->numSequences);
    curSequenceOutNum++;
    if( curSequenceOutNum >= NAN_DATA_OUT_MAX )
    {
        curSequenceOutNum = 0;
    }
    pDataOut->pFrameArrayHead = malloc( sizeof( NNSG2dAnimFrameData ) * pDataOut->numTotalFrames );

    NNSG2dAnimSequenceData*   pSeq            = pDataOut->pSequenceArrayHead;
    WIN_NNSG2dAnimSequenceData * pSeqWin        = (void *)((u64)pDataWin + (u64)pDataWin->pSequenceArrayHead );
    NNSG2dAnimFrameData*      pFrameBase      = pDataOut->pFrameArrayHead;

    WIN_NNSG2dAnimFrameData * pFrameDataWin;

    u32 curFrames = 0;

    for( i = 0; i < pDataOut->numSequences; i++ )
    {
        pSeq[i].numFrames = pSeqWin[i].numFrames;
        pSeq[i].loopStartFrameIdx = pSeqWin[i].loopStartFrameIdx;
        pSeq[i].animType = pSeqWin[i].animType;
        pSeq[i].playMode = pSeqWin[i].playMode;
        pSeq[i].pAnmFrameArray = (NNSG2dAnimFrameData*)((void*)pDataOut->pFrameArrayHead + curFrames * sizeof( NNSG2dAnimFrameData ));
        curFrames += pSeq[i].numFrames;

        pFrameDataWin = (void *)((u64)pDataWin + (u64)pDataWin->pFrameArrayHead + (u64)pSeqWin[i].pAnmFrameArray);

        for( j = 0; j < pSeq[i].numFrames; j++ )
        {
            pSeq[i].pAnmFrameArray[j].pContent = (void *)((u64)pDataWin + (u64)pDataWin->pAnimContents + (u64)pFrameDataWin[j].pContent);
            pSeq[i].pAnmFrameArray[j].frames = pFrameDataWin[j].frames;
            pSeq[i].pAnmFrameArray[j].pad16 = pFrameDataWin[j].pad16;
        }
    }

    if( pDataWin->pExtendedData != 0 )
    {
        WIN_UAATData * uaatOut = &uaatDataOuts[curUaatDataOutNum];


        pDataOut->pExtendedData = (void*)(uaatOut);

        NNSG2dUserExDataBlock* pExBlk 
                = (NNSG2dUserExDataBlock*)pDataOut->pExtendedData;
        NNSG2dUserExDataBlock* pExBlkWin
                = (NNSG2dUserExDataBlock*)((u64)pDataWin + (u64)pDataWin->pExtendedData);
        pExBlk->blkTypeID = pExBlkWin->blkTypeID;
        pExBlk->blkSize = pExBlkWin->blkSize;
        WIN_NNSG2dUserExAnimAttrBank* pAnmExAttrBankWin 
                = (WIN_NNSG2dUserExAnimAttrBank*)(pExBlkWin + 1);
        NNSG2dUserExAnimAttrBank * pAnmExAttrBank = &uaatOut->exAttrBank;
        pAnmExAttrBank->numSequences = pAnmExAttrBankWin->numSequences;
        pAnmExAttrBank->numAttribute = pAnmExAttrBankWin->numAttribute;

        WIN_NNSG2dUserExAnimSequenceAttr * pAnmSeqAttrArrayWin;
        NNSG2dUserExAnimSequenceAttr * pAnmSeqAtttrArray;
        pAnmSeqAttrArrayWin = (WIN_NNSG2dUserExAnimSequenceAttr*)((u64)pAnmExAttrBankWin + (u64)pAnmExAttrBankWin->pAnmSeqAttrArray);
        pAnmExAttrBank->pAnmSeqAttrArray = (NNSG2dUserExAnimSequenceAttr *)userExAnimSeqAttrOuts[curUaatDataOutNum];

        for( i=0; i < pAnmExAttrBank->numSequences; i++ ) {
            NNSG2dUserExAnimSequenceAttr* pSeqAttr =  &pAnmExAttrBank->pAnmSeqAttrArray[i];
            WIN_NNSG2dUserExAnimSequenceAttr* pSeqAttrWin = &pAnmSeqAttrArrayWin[i];
            pSeqAttr->numFrames = pSeqAttrWin->numFrames;
            pSeqAttr->pad16 = pSeqAttrWin->pad16;
            pSeqAttr->pAttr = (u32*)((u64)pAnmExAttrBankWin + (u64)pSeqAttrWin->pAttr);
            pSeqAttr->pAnmFrmAttrArray = malloc( sizeof(NNSG2dUserExAnimFrameAttr) * pSeqAttr->numFrames);
            for( j = 0; j < pSeqAttr->numFrames; j++ ) {
                NNSG2dUserExAnimFrameAttr * pFrm = &pSeqAttr->pAnmFrmAttrArray[j];
                WIN_NNSG2dUserExAnimFrameAttr * pFrmArrWin = (void*)((u64)pAnmExAttrBankWin + (u64)pSeqAttrWin->pAnmFrmAttrArray);
                WIN_NNSG2dUserExAnimFrameAttr * pFrmWin = &pFrmArrWin[j];
                pFrm->pAttr = (void*)((u64)pAnmExAttrBankWin + (u64)pFrmWin->pAttr);
            }
        }
       

        curUaatDataOutNum++;
        if( curUaatDataOutNum >= NAN_UAAT_DATA_OUT_MAX )
        {
            curUaatDataOutNum = 0;
        }
    }

    curDataOutNum++;
    if( curDataOutNum >= NAN_DATA_OUT_MAX )
    {
        curDataOutNum = 0;
    }
    return pDataOut;
    #else
    pData->pSequenceArrayHead = NNS_G2D_UNPACK_OFFSET_PTR(pData->pSequenceArrayHead, pData);
    pData->pFrameArrayHead = NNS_G2D_UNPACK_OFFSET_PTR(pData->pFrameArrayHead, pData);
    pData->pAnimContents = NNS_G2D_UNPACK_OFFSET_PTR(pData->pAnimContents, pData);

    {
        NNSG2dAnimSequenceData * pSeq = pData->pSequenceArrayHead;
        NNSG2dAnimFrameData * pFrameBase = pData->pFrameArrayHead;
        void * pContentsBase = pData->pAnimContents;

        for (i = 0; i < pData->numSequences; i++) {
            pSeq[i].pAnmFrameArray = NNS_G2D_UNPACK_OFFSET_PTR(pSeq[i].pAnmFrameArray, pFrameBase);

            for (j = 0; j < pSeq[i].numFrames; j++) {
                pSeq[i].pAnmFrameArray[j].pContent =
                    NNS_G2D_UNPACK_OFFSET_PTR(pSeq[i].pAnmFrameArray[j].pContent, pContentsBase);
            }

            NNS_G2D_ASSERTMSG(CheckAnimSequenceValidity_(&pSeq[i]), "An invalid anim-sequence is detected.");
        }
    }

    if (pData->pExtendedData != NULL) {
        pData->pExtendedData = NNS_G2D_UNPACK_OFFSET_PTR(pData->pExtendedData, pData);
        {
            u32 i = 0;
            u32 j = 0;

            NNSG2dUserExDataBlock * pExBlk = (NNSG2dUserExDataBlock *)pData->pExtendedData;
            NNSG2dUserExAnimAttrBank * pAnmExAttrBank = (NNSG2dUserExAnimAttrBank *)(pExBlk + 1);
            pAnmExAttrBank->pAnmSeqAttrArray = NNS_G2D_UNPACK_OFFSET_PTR(pAnmExAttrBank->pAnmSeqAttrArray, pAnmExAttrBank);

            for (i = 0; i < pAnmExAttrBank->numSequences; i++) {
                NNSG2dUserExAnimSequenceAttr * pSeqAttr = &pAnmExAttrBank->pAnmSeqAttrArray[i];

                pSeqAttr->pAttr = NNS_G2D_UNPACK_OFFSET_PTR(pSeqAttr->pAttr, pAnmExAttrBank);
                pSeqAttr->pAnmFrmAttrArray = NNS_G2D_UNPACK_OFFSET_PTR(pSeqAttr->pAnmFrmAttrArray, pAnmExAttrBank);
                for (j = 0; j < pSeqAttr->numFrames; j++) {
                    NNSG2dUserExAnimFrameAttr * pFrm = &pSeqAttr->pAnmFrmAttrArray[j];
                    pFrm->pAttr = NNS_G2D_UNPACK_OFFSET_PTR(pFrm->pAttr, pAnmExAttrBank);
                }
            }
        }
    }
    #endif

    NNSI_G2D_DEBUGMSG0("Unpacking NANR file is successful.\n");
}

const NNSG2dAnimSequenceData * NNS_G2dGetAnimSequenceByIdx (const NNSG2dAnimBankData * pAnimBank, u16 idx)
{
    NNS_G2D_NULL_ASSERT(pAnimBank);

    if (NNS_G2dGetNumAnimSequence(pAnimBank) > idx) {
        return &pAnimBank->pSequenceArrayHead[idx];
    } else {
        return NULL;
    }
}

#ifndef SDK_FINALROM
static void PrintAnimUserExAttr_ (const void * pUserEx)
{
    u32 i = 0;
    u32 j = 0;
    u32 k = 0;

    NNSG2dUserExDataBlock * pExBlk = (NNSG2dUserExDataBlock *)pUserEx;
    NNSG2dUserExAnimAttrBank * pAnmExAttrBank = (NNSG2dUserExAnimAttrBank *)(pExBlk + 1);

    OS_Printf("------- NNSG2dUserExAnimAttrBank -------\n");
    OS_Printf("numAttribute = %d\n", pAnmExAttrBank->numAttribute);
    OS_Printf("numSequences = %d\n", pAnmExAttrBank->numSequences);

    for (i = 0; i < pAnmExAttrBank->numSequences; i++) {
        NNSG2dUserExAnimSequenceAttr * pSeqAttr = &pAnmExAttrBank->pAnmSeqAttrArray[i];
        OS_Printf("------- NNSG2dUserExAnimSequenceAttr -------\n");
        OS_Printf("numFrames = %d\n", pSeqAttr->numFrames);

        for (k = 0; k < pAnmExAttrBank->numAttribute; k++) {
            OS_Printf("seq_attr( %03d, %d )   = %08x\n", i, k, pSeqAttr->pAttr[0]);
        }

        for (j = 0; j < pSeqAttr->numFrames; j++) {
            NNSG2dUserExAnimFrameAttr * pFrm = &pSeqAttr->pAnmFrmAttrArray[j];
            for (k = 0; k < pAnmExAttrBank->numAttribute; k++) {
                OS_Printf("frame_attr( %03d, %d ) = %08x\n", j, k, pFrm->pAttr[0]);
            }
        }
        OS_Printf("-------------------------------------------\n");
    }
    OS_Printf("---------------------------------------\n");
}

void NNS_G2dPrintAnimContents (const void * pData, NNSG2dAnimationElement animElem)
{
    NNS_G2D_NULL_ASSERT(pData);

    switch (animElem) {
    case NNS_G2D_ANIMELEMENT_INDEX:
    {
        const NNSG2dAnimData * pAnimData = (const NNSG2dAnimData *)pData;
        OS_Printf("index = %d\n", *pAnimData);
        break;
    }
    case NNS_G2D_ANIMELEMENT_INDEX_SRT:
    {
        const NNSG2dAnimDataSRT * pAnimData = (const NNSG2dAnimDataSRT *)pData;
        OS_Printf("index = %d\n", pAnimData->index);

        OS_Printf("rotZ = %x\n", pAnimData->rotZ);

        OS_Printf("sx = %f\n", FX_FX32_TO_F32(pAnimData->sx));
        OS_Printf("sy = %f\n", FX_FX32_TO_F32(pAnimData->sy));

        OS_Printf("px = %d\n", pAnimData->px);
        OS_Printf("py = %d\n", pAnimData->py);
        break;
    }
    case NNS_G2D_ANIMELEMENT_INDEX_T:
    {
        const NNSG2dAnimDataT * pAnimData = (const NNSG2dAnimDataT *)pData;
        OS_Printf("index = %d\n", pAnimData->index);

        OS_Printf("px = %d\n", pAnimData->px);
        OS_Printf("py = %d\n", pAnimData->py);
        break;
    }
    default:
        NNS_G2D_ASSERT(FALSE);
    }
}

void NNS_G2dPrintAnimFrame (const NNSG2dAnimFrameData * pFrame, NNSG2dAnimationElement animElem)
{
    NNS_G2D_NULL_ASSERT(pFrame);

    OS_Printf("frames = %d\n", pFrame->frames);
    NNS_G2dPrintAnimContents(pFrame->pContent, animElem);
}

void NNS_G2dPrintAnimSequence (const NNSG2dAnimSequenceData * pSeq)
{
    u16 i;
    NNS_G2D_NULL_ASSERT(pSeq);
    {
        const NNSG2dAnimationType animType = NNS_G2dGetAnimSequenceAnimType(pSeq);
        const NNSG2dAnimationElement animElem = NNS_G2dGetAnimSequenceElementType(pSeq);

        NNS_G2D_MINMAX_ASSERT(animType, NNS_G2D_ANIMATIONTYPE_CELL, NNS_G2D_ANIMATIONTYPE_MAX);
        NNS_G2D_MINMAX_ASSERT(animElem, NNS_G2D_ANIMELEMENT_INDEX, NNS_G2D_ANIMELEMENT_INDEX_T);

        OS_Printf("-------------Sequence---------------\n");
        OS_Printf("numFrames = %d\n", pSeq->numFrames);
        OS_Printf("animType  = %s\n", s_animTypeStrTbl[ animType ]);
        OS_Printf("animElem  = %s\n", s_elemTypeStrTbl[ animElem ]);

        OS_Printf("loopStartFrameIdx  = %d\n", pSeq->loopStartFrameIdx);
        OS_Printf("playMode           = %s\n", s_playModeStrTbl[pSeq->playMode]);

        for (i = 0; i < pSeq->numFrames; i++) {
            NNS_G2dPrintAnimFrame(&pSeq->pAnmFrameArray[i], animElem);
        }
        OS_Printf("------------------------------------\n");
    }
}

void NNS_G2dPrintAnimBank (const NNSG2dAnimBankData * pAnimBank)
{
    u16 i;

    NNS_G2D_NULL_ASSERT(pAnimBank);

    OS_Printf("---------------Anim Bank---------------------\n");
    OS_Printf("numSequences = %d\n", pAnimBank->numSequences);
    for (i = 0; i < pAnimBank->numSequences; i++) {
        NNS_G2dPrintAnimSequence(&pAnimBank->pSequenceArrayHead[i]);
    }

    if (pAnimBank->pExtendedData != NULL) {
        OS_Printf("---------------UserEx--------------------\n");
        PrintAnimUserExAttr_(pAnimBank->pExtendedData);
        OS_Printf("-----------------------------------------\n");
    }
    OS_Printf("---------------------------------------------\n");
}
#endif
