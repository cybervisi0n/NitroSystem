#include <nitro.h>

#include <nnsys/g2d/load/g2d_NCE_load.h>
#include <nnsys/g2d/g2d_Load.h>

#include "g2di_Debug.h"

#ifdef SDK_PORT
#define NOB_ALLOCATED_CELL_BANKS_MAX 2048
typedef struct {
    void * origAddr;
    void * mallocAddr;
} WIN_NOB_malloc_t;
static WIN_NOB_malloc_t s_allocatedCellBanksTbl[NOB_ALLOCATED_CELL_BANKS_MAX] = {0};

static void WIN_FreeCellBank( NNSG2dCellDataBank * pCellBank );
static void WIN_RegisterCellBank( NNSG2dCellDataBank ** ppCellBank );

static void WIN_FreeCellBank(NNSG2dCellDataBank * pCellBank)
{
    free(pCellBank->pCellDataArrayHead);
    free(pCellBank);
    return;
}

static void WIN_RegisterCellBank(NNSG2dCellDataBank ** ppCellBank )
{
    for(int i=0; i < NOB_ALLOCATED_CELL_BANKS_MAX; i++)
    {
        if(s_allocatedCellBanksTbl[i].origAddr == NULL)
        {
            s_allocatedCellBanksTbl[i].origAddr = ppCellBank;
            s_allocatedCellBanksTbl[i].mallocAddr = *ppCellBank;
            break;
        }
    }
}

void WIN_CheckAndFreeCellBank(void * start, void * end)
{
    u64 start64 = (u64)start;
    u64 end64 = (u64)end;
    for(int i=0; i < NOB_ALLOCATED_CELL_BANKS_MAX; i++)
    {
        if((u64)s_allocatedCellBanksTbl[i].origAddr > start64 && (u64)s_allocatedCellBanksTbl[i].origAddr < end64)
        {
            NNSG2dCellDataBank * bank = s_allocatedCellBanksTbl[i].mallocAddr;
            WIN_FreeCellBank(bank);
            s_allocatedCellBanksTbl[i].origAddr = NULL;
            s_allocatedCellBanksTbl[i].mallocAddr = NULL;
            break;
        }
    }
}
#endif

static void * GetPtrOamArrayHead_ (NNSG2dCellDataBank * pCellBank)
{
    NNS_G2D_NULL_ASSERT(pCellBank);

    if (NNSi_G2dCellBankHasBR(pCellBank)) {
        return (NNSG2dCellDataWithBR *)(pCellBank->pCellDataArrayHead) + pCellBank->numCells;
    } else {
        return pCellBank->pCellDataArrayHead + pCellBank->numCells;
    }
}

static void UnPackExtendedData_ (void * pExData)
{
    NNS_G2D_NULL_ASSERT(pExData);
    {
        NNSG2dUserExDataBlock * pBlk = (NNSG2dUserExDataBlock *)pExData;
        NNSG2dUserExCellAttrBank * pCellAttrBank = (NNSG2dUserExCellAttrBank *)(pBlk + 1);

        NNSi_G2dUnpackUserExCellAttrBank(pCellAttrBank);
    }
}

BOOL NNS_G2dGetUnpackedCellBank (void * pNcerFile, NNSG2dCellDataBank ** ppCellBank)
{
    NNS_G2D_NULL_ASSERT(pNcerFile);
    NNS_G2D_NULL_ASSERT(ppCellBank);

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileSignatureValid(pNcerFile,
                                                      NNS_G2D_BINFILE_SIG_CELL),
                      "Input file signature is invalid for this method.");

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileVersionValid(pNcerFile,
                                                    BIN_FILE_VERSION(NCER)),
                      "Input file is obsolete. Please use the new g2dcvtr.exe.");

    {
        NNSG2dBinaryFileHeader * pBinFile = (NNSG2dBinaryFileHeader *)pNcerFile;
        {
            NNSG2dCellDataBankBlock * pBinBlk =
                (NNSG2dCellDataBankBlock *)NNS_G2dFindBinaryBlock(pBinFile,
                                                                  NNS_G2D_BLKSIG_CELLBANK);
            if (pBinBlk) {
                #ifdef SDK_PORT
                *ppCellBank = NNS_G2dUnpackNCE( (void*)&pBinBlk->cellDataBank );
                WIN_RegisterCellBank(ppCellBank);
                return TRUE;
                #else
                NNS_G2dUnpackNCE((void *)&pBinBlk->cellDataBank);
                *ppCellBank = &pBinBlk->cellDataBank;
                return TRUE;
                #endif
            } else {
                *ppCellBank = NULL;
                return FALSE;

            }
        }
    }
}

const NNSG2dCellData * NNS_G2dGetCellDataByIdx (const NNSG2dCellDataBank * pCellData, u16 idx)
{
    NNS_G2D_NULL_ASSERT(pCellData);

    if (idx >= pCellData->numCells) {
        return NULL;
    }

    if (NNSi_G2dCellBankHasBR(pCellData)) {
        const NNSG2dCellDataWithBR * pCellBR =
            (const NNSG2dCellDataWithBR *)(pCellData->pCellDataArrayHead) + idx;
        return &pCellBR->cellData;
    } else {
        return pCellData->pCellDataArrayHead + idx;
    }
}

#ifdef SDK_PORT
#define NOB_CELL_DATA_MAX 256
#define NOB_CELL_ARRAY_SIZE_MAX 1024
#define NOB_USER_EX_DATA_OUT_MAX NOB_CELL_DATA_MAX
#define NOB_USER_EX_DATA_ATTR_MAX 128
static NNSG2dCellDataBank cellDataOuts[NOB_CELL_DATA_MAX] = {0};
static u32 curCellDataOut = 0;
static u8 cellDataArrayOuts[NOB_CELL_DATA_MAX][sizeof(NNSG2dCellData) * NOB_CELL_ARRAY_SIZE_MAX] = {0};
static NNSG2dVramTransferData vramTransferOuts[NOB_CELL_DATA_MAX] = {0};
static u32 curVramTransferOut = 0;

typedef struct WIN_NOBUserExData
{
    NNSG2dUserExDataBlock blk;
    NNSG2dUserExCellAttrBank cellAttrBank;
}WIN_NOBUserExData;

static WIN_NOBUserExData userExDataOuts[NOB_USER_EX_DATA_OUT_MAX];
static u8 userExAttrArrayOuts[NOB_USER_EX_DATA_OUT_MAX][sizeof(NNSG2dUserExCellAttr) * NOB_USER_EX_DATA_ATTR_MAX];
static u32 curUserExDataOut = 0;


NNSG2dCellDataBank* NNS_G2dUnpackNCE( NNSG2dCellDataBank* pCellData )
#else
void NNS_G2dUnpackNCE (NNSG2dCellDataBank * pCellData)
#endif
{
    NNS_G2D_NULL_ASSERT(pCellData);

    #ifdef SDK_PORT
    NNSG2dCellDataBank * pCellDataOut;
    WIN_NNSG2dCellDataBank * pCellDataWin;
    pCellDataWin = (WIN_NNSG2dCellDataBank *)pCellData;
    pCellDataOut = malloc( sizeof( NNSG2dCellDataBank ) );
    //pCellDataOut = &cellDataOuts[curCellDataOut];

    memset( pCellDataOut, 0, sizeof( NNSG2dCellDataBank ) );

    pCellDataOut->numCells = pCellDataWin->numCells;
    pCellDataOut->cellBankAttr = pCellDataWin->cellBankAttr;
    pCellDataOut->mappingMode = pCellDataWin->mappingMode;

    //pCellDataOut->pCellDataArrayHead = (void*)((u64)pCellDataOut + (u64)pCellDataWin->pCellDataArrayHead);
    pCellDataOut->pCellDataArrayHead = malloc(sizeof(NNSG2dCellDataWithBR) * pCellDataOut->numCells);
    //pCellDataOut->pCellDataArrayHead = (void *)cellDataArrayOuts[curCellDataOut];

    curCellDataOut++;
    if(curCellDataOut >= NOB_CELL_DATA_MAX )
    {
        curCellDataOut = 0;
    }

    WIN_NNSG2dCellData * pCellWin;
    WIN_NNSG2dCellDataWithBR * pCellWinBr;

    WIN_NNSG2dCellDataBank* pCellDataArrayHeadWin = NULL;
    pCellDataArrayHeadWin = (void*)((u64)pCellDataWin + (u64)pCellDataWin->pCellDataArrayHead);

    void* pHeadOfOAMData;

    if(pCellDataOut->cellBankAttr & NNS_G2D_CELLBK_ATTR_CELLWITHBR)
    {
        pCellWinBr = (WIN_NNSG2dCellDataWithBR *)((u64)pCellDataArrayHeadWin);
        //pHeadOfOAMData = (void*)(&pCellWinBr->cellData) + 4;
        pHeadOfOAMData = pCellWinBr + pCellDataWin->numCells;
        //pHeadOfOAMData = &pCellWinBr->cellData + pCellDataWin->numCells;
    }
    else
    {
        pHeadOfOAMData = (WIN_NNSG2dCellData *)pCellDataArrayHeadWin + pCellDataWin->numCells;
    }

    u32 i;
    NNSG2dCellData* pCellOut = NULL;
    
    
    void * pCellWinPtr = NULL;

    for( i=0; i < pCellDataOut->numCells; i++ )
    {
        if(pCellDataOut->cellBankAttr & NNS_G2D_CELLBK_ATTR_CELLWITHBR)
        {
            pCellWinBr = (WIN_NNSG2dCellDataWithBR *)((u64)pCellDataArrayHeadWin) + i;
            pCellWin = (WIN_NNSG2dCellData *)pCellWinBr;
            


        } else {
            pCellWin = (WIN_NNSG2dCellData *)((u64)pCellDataArrayHeadWin) + i;
        }
        pCellOut = (NNSG2dCellData*)(NNS_G2dGetCellDataByIdx( pCellDataOut, i ));
        pCellOut->numOAMAttrs = pCellWin->numOAMAttrs;
        pCellOut->cellAttr = pCellWin->cellAttr;
        pCellOut->pOamAttrArray = (void *)((u64)pHeadOfOAMData + (u64)pCellWin->pOamAttrArray);

        if(pCellDataOut->cellBankAttr & NNS_G2D_CELLBK_ATTR_CELLWITHBR)
        {
            NNSG2dCellDataWithBR * pCellOutBr;
            pCellOutBr = (NNSG2dCellDataWithBR*)pCellOut;
            pCellOutBr->boundingRect.maxX = pCellWinBr->boundingRect.maxX;
            pCellOutBr->boundingRect.maxY = pCellWinBr->boundingRect.maxY;
            pCellOutBr->boundingRect.minX = pCellWinBr->boundingRect.minX;
            pCellOutBr->boundingRect.minY = pCellWinBr->boundingRect.minY;
        }

    //    //pCellOut->pOamAttrArray = (void *)((u64)
    //    
    }

    if( pCellDataWin->pVramTransferData != 0 )
    {
        WIN_NNSG2dVramTransferData * pVramTsfmDataWin;
        NNSG2dVramTransferData* pVramTsfmDataOut;
        pVramTsfmDataWin = (void *)((u64)pCellDataWin + (u64)pCellDataWin->pVramTransferData);
        //pVramTsfmDataOut = malloc( sizeof( NNSG2dVramTransferData ) );
        pVramTsfmDataOut = &vramTransferOuts[curVramTransferOut];
        curVramTransferOut++;
        if(curVramTransferOut >= NOB_CELL_DATA_MAX )
        {
        curVramTransferOut = 0;
        }

        pVramTsfmDataOut->szByteMax = pVramTsfmDataWin->szByteMax;
        pVramTsfmDataOut->pCellTransferDataArray = (void *)((u64)pVramTsfmDataWin + (u64)pVramTsfmDataWin->pCellTransferDataArray);
        pCellDataOut->pVramTransferData = pVramTsfmDataOut;
    }

    if( pCellDataWin->pExtendedData != 0 )
    {
        {
            NNSG2dUserExDataBlock * pBlkWin = (NNSG2dUserExDataBlock*)((u64)pCellDataWin + (u64)pCellDataWin->pExtendedData);
            WIN_NNSG2dUserExCellAttrBank * pCellAttrBankWin;
            pCellAttrBankWin = (WIN_NNSG2dUserExCellAttrBank*)(pBlkWin+1);
            WIN_NNSG2dUserExCellAttr * pCellAttrArrayWin;
            pCellAttrArrayWin = (WIN_NNSG2dUserExCellAttr *)((u64)pCellAttrBankWin + pCellAttrBankWin->pCellAttrArray);
            pCellDataOut->pExtendedData = &userExDataOuts[curUserExDataOut];

            NNSG2dUserExDataBlock * pBlk;
            pBlk = (NNSG2dUserExDataBlock *)pCellDataOut->pExtendedData;
            pBlk->blkTypeID = pBlkWin->blkTypeID;
            pBlk->blkSize = pBlkWin->blkSize;
            
            NNSG2dUserExCellAttrBank * pCellAttrBank;
            pCellAttrBank = (NNSG2dUserExCellAttrBank *)(pBlk + 1);
            pCellAttrBank->numCells = pCellAttrBankWin->numCells;
            pCellAttrBank->numAttribute = pCellAttrBankWin->numAttribute;

            NNSG2dUserExCellAttr * pCellAttrArray;
            pCellAttrArray = (NNSG2dUserExCellAttr *)(&userExAttrArrayOuts[curUserExDataOut]);
            
            pCellAttrBank->pCellAttrArray = pCellAttrArray;

            for( i=0; i < pCellAttrBank->numCells; i++ ) {
                pCellAttrArray[i].pAttr = (u32 *)((u64)pCellAttrBankWin + (u64)pCellAttrArrayWin[i].pAttr);
            }
            curUserExDataOut++;
            if( curUserExDataOut > NOB_USER_EX_DATA_OUT_MAX )
            {
                curUserExDataOut = 0;
            }
        }
    }

    return pCellDataOut;
    #else
    {
        pCellData->pCellDataArrayHead = NNS_G2D_UNPACK_OFFSET_PTR(pCellData->pCellDataArrayHead, pCellData);

        {
            void * pHeadOfOAMData = GetPtrOamArrayHead_(pCellData);

            u16 i;
            NNSG2dCellData * pCell = NULL;
            for (i = 0; i < pCellData->numCells; i++) {
                pCell = (NNSG2dCellData *)(NNS_G2dGetCellDataByIdx(pCellData, i));
                pCell->pOamAttrArray = NNS_G2D_UNPACK_OFFSET_PTR(pCell->pOamAttrArray, pHeadOfOAMData);
            }
        }

        if (pCellData->pVramTransferData != NULL) {
            NNSG2dVramTransferData * pVramTsfmData = NNS_G2D_UNPACK_OFFSET_PTR(pCellData->pVramTransferData, pCellData);

            pVramTsfmData->pCellTransferDataArray = NNS_G2D_UNPACK_OFFSET_PTR(pVramTsfmData->pCellTransferDataArray, pVramTsfmData);
            pCellData->pVramTransferData = pVramTsfmData;
        }

        if (pCellData->pExtendedData != NULL) {
            pCellData->pExtendedData = NNS_G2D_UNPACK_OFFSET_PTR(pCellData->pExtendedData, pCellData);
            UnPackExtendedData_(pCellData->pExtendedData);
        }

    }
    #endif

    NNSI_G2D_DEBUGMSG0("Unpacking NCER file is successful.\n");
}

#ifndef SDK_FINALROM
void NNS_G2dPrintOBJAttr (const NNSG2dCellOAMAttrData * pOBJ)
{
    NNS_G2D_NULL_ASSERT(pOBJ);
    OS_Printf("OBJ_0 = %x\n", pOBJ->attr0);
    OS_Printf("OBJ_1 = %x\n", pOBJ->attr1);
    OS_Printf("OBJ_2 = %x\n\n", pOBJ->attr2);
}

static void PrintCellBoundingRect_ (const NNSG2dCellBoundingRectS16 * pBR)
{
    NNS_G2D_NULL_ASSERT(pBR);
    OS_Printf("maxX = %d\n", pBR->maxX);
    OS_Printf("maxY = %d\n", pBR->maxY);
    OS_Printf("minX = %d\n", pBR->minX);
    OS_Printf("minY = %d\n", pBR->minY);
}

void NNS_G2dPrintCellInfo (const NNSG2dCellData * pCell)
{
    u16 i;
    NNS_G2D_NULL_ASSERT(pCell);

    OS_Printf("-------------------\n");
    {
        const NNSG2dCellOAMAttrData * pOBJ = pCell->pOamAttrArray;

        OS_Printf("numOBJ = %d\n", pCell->numOAMAttrs);

        if (NNSi_G2dCellHasBR(pCell)) {
            const NNSG2dCellDataWithBR * pCellBR
                = (const NNSG2dCellDataWithBR *)(pCell);
            PrintCellBoundingRect_(&pCellBR->boundingRect);
        }

        for (i = 0; i < pCell->numOAMAttrs; i++) {
            NNS_G2dPrintOBJAttr(&pOBJ[i]);
        }
    }
    OS_Printf("-------------------\n");
}

static void PrintVramTransformData_ (const NNSG2dVramTransferData * pVramTsfmData, u16 numCells)
{
    u16 i;
    NNS_G2D_NULL_ASSERT(pVramTsfmData);

    for (i = 0; i < numCells; i++) {
        const NNSG2dCellVramTransferData * pTsfmData
            = &pVramTsfmData->pCellTransferDataArray[i];

        OS_Printf("srcDataOffset   = %d\n", pTsfmData->srcDataOffset);
        OS_Printf("szByte          = %d\n", pTsfmData->szByte);
    }
}

static void PrintCellExtendedData_ (const void * pExData)
{
    NNS_G2D_NULL_ASSERT(pExData);
    {
        NNSG2dUserExDataBlock * pBlk = (NNSG2dUserExDataBlock *)pExData;
        NNSG2dUserExCellAttrBank * pCellAttrBank = (NNSG2dUserExCellAttrBank *)(pBlk + 1);

        NNSi_G2dPrintUserExCellAttrBank(pCellAttrBank);
    }
}

void NNS_G2dPrintCellBank (const NNSG2dCellDataBank * pCellBank)
{
    u16 i;
    const NNSG2dCellData * pCell;
    OS_Printf("---------------------------------------------\n");
    OS_Printf("numCell = %d\n", pCellBank->numCells);
    for (i = 0; i < pCellBank->numCells; i++) {
        OS_Printf("Cell Idx = %d\n", i);
        pCell = NNS_G2dGetCellDataByIdx(pCellBank, i);
        NNS_G2dPrintCellInfo(pCell);
    }

    if (pCellBank->pVramTransferData != NULL) {
        OS_Printf("--------- VramTransform Data\n");
        {
            const NNSG2dVramTransferData * pVramTsfmData
                = (const NNSG2dVramTransferData *)pCellBank->pVramTransferData;

            OS_Printf("szByteMax   = %d\n", pVramTsfmData->szByteMax);
            PrintVramTransformData_(pVramTsfmData, pCellBank->numCells);
        }
    }

    if (pCellBank->pExtendedData != NULL) {
        PrintCellExtendedData_(pCellBank->pExtendedData);
    }

    OS_Printf("---------------------------------------------\n");
}
#endif
