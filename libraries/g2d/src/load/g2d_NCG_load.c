#include <nitro.h>

#include <nnsys/g2d/load/g2d_NCG_load.h>
#include <nnsys/g2d/g2d_Load.h>

#include "g2di_Debug.h"

#define NNS_G2D_NCGR_VER_FOR_CHARDATA_BLK_LOADING   NNS_G2dMakeVersionData((u8)1, (u8)0)

#ifndef SDK_FINALROM
static const char * StrNNSG2dPixelFmt_ [] = {
    "INVALID_PixelFmt !! ",
    "INVALID_PixelFmt !! ",
    "INVALID_PixelFmt !! ",
    "GX_TEXFMT_PLTT16",
    "GX_TEXFMT_PLTT256",
    "INVALID_PixelFmt !! ",
    "INVALID_PixelFmt !! ",
    "INVALID_PixelFmt !! "
};

static const char * StrNNSG2dCharacterOrder_ [] = {
    "NNS_G2D_CHARACTER_FMT_CHAR",
    "NNS_G2D_CHARACTER_FMT_BMP",
    "NNS_G2D_CHARACTER_FMT_MAX"
};

static const char * GetStrNNSG2dCharacterDataMappingType_ (GXOBJVRamModeChar type)
{
    switch (type) {
    case GX_OBJVRAMMODE_CHAR_2D:
        return "GX_OBJVRAMMODE_CHAR_2D           ";
    case GX_OBJVRAMMODE_CHAR_1D_32K:
        return "GX_OBJVRAMMODE_CHAR_1D_32K       ";
    case GX_OBJVRAMMODE_CHAR_1D_64K:
        return "GX_OBJVRAMMODE_CHAR_1D_64K       ";
    case GX_OBJVRAMMODE_CHAR_1D_128K:
        return "GX_OBJVRAMMODE_CHAR_1D_128K      ";
    case GX_OBJVRAMMODE_CHAR_1D_256K:
        return "GX_OBJVRAMMODE_CHAR_1D_256K      ";
    default: NNS_G2D_ASSERT(FALSE);
        return "INVALID_TYPE";
    }
}
#endif

#ifdef SDK_PORT
#define NCG_DATA_OUT_MAX 256
#define NCG_DATA_OUT_MAX_SIZE 1024
static u8 curDataOut = 0;
static NNSG2dCharacterData charDataOuts[NCG_DATA_OUT_MAX] = {0};

static u8 curBGDataOut = 0;

#define NCG_ALLOCATED_CHAR_DATA_MAX 4096
typedef struct {
    void * origAddr;
    void * mallocAddr;
} WIN_NCG_malloc_t;
static WIN_NCG_malloc_t s_allocatedCharDataTbl[NCG_ALLOCATED_CHAR_DATA_MAX] = {0};

static void WIN_FreeCharData(NNSG2dCharacterData * pCharData);
static void WIN_RegisterCharData(NNSG2dCharacterData ** ppCharData );

static void WIN_FreeCharData(NNSG2dCharacterData * pCharData)
{
    free(pCharData);
    return;
}

static void WIN_RegisterCharData(NNSG2dCharacterData ** ppCharData )
{
    for(int i=0; i < NCG_ALLOCATED_CHAR_DATA_MAX; i++)
    {
        if(s_allocatedCharDataTbl[i].origAddr == NULL)
        {
            s_allocatedCharDataTbl[i].origAddr = ppCharData;
            s_allocatedCharDataTbl[i].mallocAddr = *ppCharData;
            break;
        }
    }
}

void WIN_CheckAndFreeCharData(void * start, void * end)
{
    u64 start64 = (u64)start;
    u64 end64 = (u64)end;
    for(int i=0; i < NCG_ALLOCATED_CHAR_DATA_MAX; i++)
    {
        if((u64)s_allocatedCharDataTbl[i].origAddr > start64 && (u64)s_allocatedCharDataTbl[i].origAddr < end64)
        {
            NNSG2dCharacterData * chrData = s_allocatedCharDataTbl[i].mallocAddr;
            WIN_FreeCharData(chrData);
            s_allocatedCharDataTbl[i].origAddr = NULL;
            s_allocatedCharDataTbl[i].mallocAddr = NULL;
            break;
        }
    }
}
#endif

BOOL NNS_G2dGetUnpackedCharacterData (void * pNcgrFile, NNSG2dCharacterData ** ppCharData)
{
    NNS_G2D_NULL_ASSERT(pNcgrFile);
    NNS_G2D_NULL_ASSERT(ppCharData);

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileSignatureValid(pNcgrFile,
                                                      NNS_G2D_BINFILE_SIG_CHARACTERDATA),
                      "Input file signature is invalid for this method.");

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileVersionValid(pNcgrFile,
                                                    NNS_G2D_NCGR_VER_FOR_CHARDATA_BLK_LOADING),
                      "Input file is obsolete. Please use the new g2dcvtr.exe.");

    {
        NNSG2dBinaryFileHeader * pBinFile = (NNSG2dBinaryFileHeader *)pNcgrFile;
        {
            NNSG2dCharacterDataBlock * pBinBlk =
                (NNSG2dCharacterDataBlock *)NNS_G2dFindBinaryBlock(pBinFile,
                                                                   NNS_G2D_BINBLK_SIG_CHARACTERDATA);
            if (pBinBlk) {
                #ifdef SDK_PORT
                *ppCharData = NNS_G2dUnpackNCG( (void*)&pBinBlk->characterData );
                WIN_RegisterCharData(ppCharData);
                #else
                NNS_G2dUnpackNCG((void *)&pBinBlk->characterData);
                *ppCharData = &pBinBlk->characterData;
                #endif
                return TRUE;
            } else {
                *ppCharData = NULL;
                return FALSE;
            }
        }
    }
}

#ifdef SDK_PORT
NNSG2dCharacterData * NNS_G2dUnpackNCG(NNSG2dCharacterData * pCharData)
#else
void NNS_G2dUnpackNCG (NNSG2dCharacterData * pCharData)
#endif
{
    NNS_G2D_NULL_ASSERT(pCharData);

    #ifdef SDK_PORT
    NNSG2dCharacterData * pCharDataOut;
    WIN_NNSG2dCharacterData * pCharDataWin;
    pCharDataWin = (WIN_NNSG2dCharacterData *)pCharData;
    pCharDataOut = malloc(sizeof(NNSG2dCharacterData));

    pCharDataOut->H = pCharDataWin->H;
    pCharDataOut->W = pCharDataWin->W;
    pCharDataOut->pixelFmt = pCharDataWin->pixelFmt;
    pCharDataOut->mappingType = pCharDataWin->mappingType;
    pCharDataOut->characterFmt = pCharDataWin->characterFmt;
    pCharDataOut->szByte = pCharDataWin->szByte;
    pCharDataOut->pRawData = (void *)((u64)pCharData + pCharDataWin->pRawData);

    return pCharDataOut;
    #else

    pCharData->pRawData = NNS_G2D_UNPACK_OFFSET_PTR(pCharData->pRawData, pCharData);

    NNS_G2D_MINMAX_ASSERT(pCharData->pixelFmt, GX_TEXFMT_PLTT16, GX_TEXFMT_PLTT256);
    NNS_G2D_MINMAX_ASSERT(pCharData->mappingType, GX_OBJVRAMMODE_CHAR_2D, GX_OBJVRAMMODE_CHAR_1D_256K);
    NNS_G2D_MINMAX_ASSERT(NNSi_G2dGetCharacterFmtType(pCharData->characterFmt), NNS_G2D_CHARACTER_FMT_CHAR, NNS_G2D_CHARACTER_FMT_MAX);

    if ((pCharData->characterFmt == NNS_G2D_CHARACTER_FMT_CHAR)
        && (pCharData->mappingType == GX_OBJVRAMMODE_CHAR_2D)) {
        NNS_G2D_ASSERTMSG((pCharData->pixelFmt == GX_TEXFMT_PLTT16 &&
                           pCharData->W == 32) ||
                          (pCharData->pixelFmt == GX_TEXFMT_PLTT256 &&
                           pCharData->W == 16)
                          , "Invalid OBJ character data size for 2D mapping mode.");
    }
    #endif

    NNSI_G2D_DEBUGMSG0("Unpacking NCGR file is successful.\n");
}

BOOL NNS_G2dGetUnpackedBGCharacterData (void * pNcgrFile, NNSG2dCharacterData ** ppCharData)
{
    NNS_G2D_NULL_ASSERT(pNcgrFile);
    NNS_G2D_NULL_ASSERT(ppCharData);

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileSignatureValid(pNcgrFile,
                                                      NNS_G2D_BINFILE_SIG_CHARACTERDATA),
                      "Input file signature is invalid for this method.");

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileVersionValid(pNcgrFile,
                                                    NNS_G2D_NCGR_VER_FOR_CHARDATA_BLK_LOADING),
                      "Input file is obsolete. Please use the new g2dcvtr.exe.");

    {
        NNSG2dBinaryFileHeader * pBinFile = (NNSG2dBinaryFileHeader *)pNcgrFile;
        {
            NNSG2dCharacterDataBlock * pBinBlk =
                (NNSG2dCharacterDataBlock *)NNS_G2dFindBinaryBlock(pBinFile,
                                                                   NNS_G2D_BINBLK_SIG_CHARACTERDATA);
            if (pBinBlk) {
                #ifdef SDK_PORT
                *ppCharData = NNS_G2dUnpackBGNCG( (void*)&pBinBlk->characterData );
                WIN_RegisterCharData(ppCharData);
                #else
                NNS_G2dUnpackBGNCG((void *)&pBinBlk->characterData);
                *ppCharData = &pBinBlk->characterData;
                #endif
                return TRUE;
            } else {
                *ppCharData = NULL;
                return FALSE;
            }
        }
    }
}

BOOL NNS_G2dGetUnpackedCharacterPosInfo (void * pNcgrFile, NNSG2dCharacterPosInfo ** ppCharPosInfo)
{
    NNS_G2D_NULL_ASSERT(pNcgrFile);
    NNS_G2D_NULL_ASSERT(ppCharPosInfo);

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileSignatureValid(pNcgrFile,
                                                      NNS_G2D_BINFILE_SIG_CHARACTERDATA),
                      "Input file signature is invalid for this method.");

    NNS_G2D_ASSERTMSG(NNSi_G2dIsBinFileVersionValid(pNcgrFile,
                                                    BIN_FILE_VERSION(NCGR)),
                      "Input file is obsolete. Please use the new g2dcvtr.exe.");

    {
        NNSG2dBinaryFileHeader * pBinFile = (NNSG2dBinaryFileHeader *)pNcgrFile;
        {
            NNSG2dCharacterPosInfoBlock * pBinBlk =
                (NNSG2dCharacterPosInfoBlock *)NNS_G2dFindBinaryBlock(pBinFile,
                                                                      NNS_G2D_BINBLK_SIG_CHAR_POSITION);
            if (pBinBlk) {
                *ppCharPosInfo = &pBinBlk->posInfo;
                return TRUE;
            } else {
                *ppCharPosInfo = NULL;
                return FALSE;
            }
        }
    }
}

#ifdef SDK_PORT
NNSG2dCharacterData * NNS_G2dUnpackBGNCG(NNSG2dCharacterData * pCharData)
#else
void NNS_G2dUnpackBGNCG (NNSG2dCharacterData * pCharData)
#endif
{
    NNS_G2D_NULL_ASSERT(pCharData);

    #ifdef SDK_PORT
    NNSG2dCharacterData * pCharDataOut;
    WIN_NNSG2dCharacterData * pCharDataWin;
    pCharDataWin = (WIN_NNSG2dCharacterData *)pCharData;
    pCharDataOut = malloc( sizeof(NNSG2dCharacterData) );
    curDataOut++;
    if( curDataOut >= NCG_DATA_OUT_MAX )
    {
        curDataOut = 0;
    }

    pCharDataOut->H = pCharDataWin->H;
    pCharDataOut->W = pCharDataWin->W;
    pCharDataOut->pixelFmt = pCharDataWin->pixelFmt;
    pCharDataOut->mappingType = pCharDataWin->mappingType;
    pCharDataOut->characterFmt = pCharDataWin->characterFmt;
    pCharDataOut->szByte = pCharDataWin->szByte;
    pCharDataOut->pRawData = (void *)((u64)pCharData + pCharDataWin->pRawData);

    return pCharDataOut;
    #else

    pCharData->pRawData = NNS_G2D_UNPACK_OFFSET_PTR(pCharData->pRawData, pCharData);

    NNS_G2D_MINMAX_ASSERT(pCharData->pixelFmt, GX_TEXFMT_PLTT16, GX_TEXFMT_PLTT256);
    NNS_G2D_MINMAX_ASSERT(pCharData->mappingType, GX_OBJVRAMMODE_CHAR_2D, GX_OBJVRAMMODE_CHAR_1D_256K);
    NNS_G2D_ASSERT(NNSi_G2dGetCharacterFmtType(pCharData->characterFmt) == NNS_G2D_CHARACTER_FMT_CHAR);
    #endif

    NNSI_G2D_DEBUGMSG0("Unpacking NCGR file is successful.\n");
}

#ifndef SDK_FINALROM
void NNS_G2dPrintCharacterData (const NNSG2dCharacterData * pCharData)
{
    NNS_G2D_NULL_ASSERT(pCharData);
    {
        const NNSG2dCharacterFmt charFmt = NNSi_G2dGetCharacterFmtType(pCharData->characterFmt);

        OS_Printf("---------------------------------------------\n");
        OS_Printf(" Character Data (NCG) \n");
        OS_Printf(" szByte         = %d \n", pCharData->szByte);

        {
            if (pCharData->W == NNS_G2D_1D_MAPPING_CHAR_SIZE) {
                OS_Printf(" W              = %s \n", "Not used( 1D mapping data.)");
            } else {
                OS_Printf(" W              = %d \n", pCharData->W);
            }

            if (pCharData->H == NNS_G2D_1D_MAPPING_CHAR_SIZE) {
                OS_Printf(" H              = %s \n", "Not used( 1D mapping data.)");
            } else {
                OS_Printf(" H              = %d \n", pCharData->H);
            }
        }

        OS_Printf(" pixelFmt       = %s \n", StrNNSG2dPixelFmt_[ pCharData->pixelFmt ]);
        OS_Printf(" mapingType     = %s \n", GetStrNNSG2dCharacterDataMappingType_(pCharData->mappingType));
        OS_Printf(" characterFmt   = %s \n", StrNNSG2dCharacterOrder_[ charFmt ]);
        OS_Printf(" isVramTransfer = %d \n", NNSi_G2dIsCharacterVramTransfered(pCharData->characterFmt));
        OS_Printf("---------------------------------------------\n");
    }
}

void NNS_G2dPrintCharacterPosInfo (const NNSG2dCharacterPosInfo * pPosInfo)
{
    NNS_G2D_NULL_ASSERT(pPosInfo);
    {
        OS_Printf("---------------------------------------------\n");
        OS_Printf(" Character PosInfo (NCG) \n");
        OS_Printf(" srcPosX    = %d \n", pPosInfo->srcPosX);
        OS_Printf(" srcPosY    = %d \n", pPosInfo->srcPosY);
        OS_Printf(" srcW       = %d \n", pPosInfo->srcW);
        OS_Printf(" srcH       = %d \n", pPosInfo->srcH);
        OS_Printf("---------------------------------------------\n");
    }
}
#endif
