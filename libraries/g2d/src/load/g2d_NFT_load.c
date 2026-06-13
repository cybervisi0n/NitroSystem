
#include <nitro.h>

#include <nnsys/g2d/g2d_Load.h>
#include <nnsys/g2d/load/g2d_NFT_load.h>
#include <nnsys/g2d/g2d_config.h>

#ifdef SDK_PORT
#define NNS_FONT_INFO_OUT_MAX 100
#define NNS_FONT_WIDTH_OUT_MAX 1000
#define NNS_FONT_MAP_OUT_MAX 1000
static u32 curFontInfoOut = 0;
static u32 curFontWidthOut = 0;
static u32 curFontMapOut = 0;
static NNSG2dFontInformation fontInfoOut[NNS_FONT_INFO_OUT_MAX];
static NNSG2dFontWidth fontWidthOut[NNS_FONT_INFO_OUT_MAX];
static NNSG2dFontCodeMap fontMapOut[NNS_FONT_MAP_OUT_MAX];
#endif

BOOL NNSi_G2dGetUnpackedFont (void * pNftrFile, NNSG2dFontInformation ** ppRes)
{
#ifdef NNS_G2D_FONT_USE_OLD_RESOURCE
    BOOL isOldVer = FALSE;
#endif
    NNS_G2D_POINTER_ASSERT(pNftrFile);
    NNS_G2D_ALIGN_ASSERT(pNftrFile, 4);
    NNS_G2D_POINTER_ASSERT(ppRes);
#ifdef NNS_G2D_FONT_USE_OLD_RESOURCE
    if (!NNS_G2dIsBinFileValid(pNftrFile, NNS_G2D_BINFILE_SIG_FONTDATA, NNS_G2D_NFTR_VER)) {
        if (!NNS_G2dIsBinFileValid(pNftrFile, NNS_G2D_BINFILE_SIG_FONTDATA, NNS_G2D_NFTR_PREV_VER)) {
            OS_Panic("Input file is invalid or obsolete. Please use latest fontcvtr.");
        }

        isOldVer = TRUE;
    }
#else
    NNS_G2D_ASSERTMSG(NNS_G2dIsBinFileValid(pNftrFile, NNS_G2D_BINFILE_SIG_FONTDATA,
                                            BIN_FILE_VERSION(NFTR)), "Input file is obsolete. Please use latest fontcvtr.");
#endif

    {
        NNSG2dBinaryFileHeader * pBinFile = (NNSG2dBinaryFileHeader *)pNftrFile;
        NNSG2dBinaryBlockHeader * pBinBlock;

        #ifdef SDK_PORT
        *ppRes = NNSi_G2dUnpackNFT( pBinFile );
        #else
        NNSi_G2dUnpackNFT(pBinFile);

        pBinBlock = NNS_G2dFindBinaryBlock(pBinFile, NNS_G2D_BINBLK_SIG_FINFDATA);

        if (pBinBlock == NULL) {
            *ppRes = NULL;
            return FALSE;
        }

        *ppRes = (NNSG2dFontInformation *)((u8 *)pBinBlock + sizeof(*pBinBlock));
        #endif
#ifdef NNS_G2D_FONT_USE_OLD_RESOURCE
        if (isOldVer) {
            (*ppRes)->pGlyph->flags = 0;
        }
#endif
#ifndef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
        NNS_G2D_WARNING((*ppRes)->pGlyph->flags == 0, "Loading NFTR requires DIRECTION_SUPPORT, but it disabled.");
#endif
    }

    return TRUE;
}

static void NNS_G2D_INLINE ResolveOffset (void ** ppOffset, void * pBase)
{
    #ifdef SDK_PORT
    *ppOffset = (void *)(*(u64 *)ppOffset + (u64)pBase);
    #else
    *ppOffset = (void *)(*(u32 *)ppOffset + (u32)pBase);
    #endif
}

#ifdef SDK_PORT
void * NNSi_G2dUnpackNFT (NNSG2dBinaryFileHeader* pHeader)
#else
void NNSi_G2dUnpackNFT (NNSG2dBinaryFileHeader * pHeader)
#endif
{
    NNSG2dBinaryBlockHeader * pBlock;
    #ifdef SDK_PORT
    NNSG2dFontInformation* pInfo = &fontInfoOut[curFontInfoOut];
    curFontInfoOut++;
    if(curFontInfoOut > NNS_FONT_INFO_OUT_MAX) {
        curFontInfoOut = 0;
    }
    #else
    NNSG2dFontInformation * pInfo = NULL;
    #endif

    NNS_G2D_POINTER_ASSERT(pHeader);

    {
        int nBlocks = 0;
        pBlock = (NNSG2dBinaryBlockHeader *)((u8 *)pHeader + pHeader->headerSize);

        while (nBlocks < pHeader->dataBlocks) {
            switch (pBlock->kind) {
            case NNS_G2D_BINBLK_SIG_FINFDATA:
                NNS_G2D_ASSERT(pInfo == NULL);
                {
                    #ifdef SDK_PORT
                    WIN_NNSG2dFontInformation * pInfoWin = (WIN_NNSG2dFontInformation*)((u8*)pBlock + sizeof(*pBlock));
                    pInfo->fontType = pInfoWin->fontType;
                    pInfo->alterCharIndex = pInfoWin->alterCharIndex;
                    memcpy(&pInfo->defaultWidth, &pInfoWin->defaultWidth, sizeof(NNSG2dCharWidths));
                    pInfo->pWidth = (void*)pInfoWin->pWidth;
                    pInfo->encoding = pInfoWin->encoding;
                    pInfo->linefeed = pInfoWin->linefeed;
                    pInfo->pGlyph = (void*)pInfoWin->pGlyph;
                    pInfo->pMap = (void*)pInfoWin->pMap;
                    #else
                    pInfo = (NNSG2dFontInformation *)((u8 *)pBlock + sizeof(*pBlock));
                    #endif

                    NNS_G2D_ASSERT(pInfo->fontType == NNS_G2D_FONTTYPE_GLYPH);

                    NNS_G2D_NULL_ASSERT(pInfo->pGlyph);
                    ResolveOffset((void **)&(pInfo->pGlyph), pHeader);
                    NNS_G2D_POINTER_ASSERT(pInfo->pGlyph)

                    if (pInfo->pWidth != NULL) {
                        ResolveOffset((void **)&(pInfo->pWidth), pHeader);
                        #ifdef SDK_PORT
                        NNSG2dFontWidth * baseWidth = pInfo->pWidth;
                        NNSG2dFontWidth * curWidth = baseWidth;
                        NNSG2dFontWidth * widthOut = &fontWidthOut[curFontWidthOut];
                        pInfo->pWidth = widthOut;
                        curFontWidthOut++;
                        if(curFontWidthOut > NNS_FONT_WIDTH_OUT_MAX) {
                            curFontWidthOut = 0;
                        }
                        while(curWidth != NULL) {
                            WIN_NNSG2dFontWidth * winWidth = (WIN_NNSG2dFontWidth*)curWidth;
                            widthOut->indexBegin = winWidth->indexBegin;
                            widthOut->indexEnd = winWidth->indexEnd;
                            widthOut->widthTable = winWidth->widthTable;

                            if(winWidth->pNext != 0) {
                                widthOut->pNext = &fontWidthOut[curFontWidthOut];
                                widthOut = &fontWidthOut[curFontWidthOut];
                                curFontWidthOut++;
                                if(curFontWidthOut > NNS_FONT_WIDTH_OUT_MAX) {
                                    curFontWidthOut = 0;
                                }
                                curWidth = (void*)((u64)pHeader + (u64)winWidth->pNext);
                            } else {
                                curWidth = NULL;
                                widthOut->pNext = NULL;
                            }
                        }
                        #endif
                        NNS_G2D_POINTER_ASSERT(pInfo->pWidth)
                    }
                    if (pInfo->pMap != NULL) {
                        ResolveOffset((void **)&(pInfo->pMap), pHeader);
                        #ifdef SDK_PORT
                        NNSG2dFontCodeMap * baseMap = pInfo->pMap;
                        NNSG2dFontCodeMap * curMap = baseMap;
                        NNSG2dFontCodeMap * mapOut = &fontMapOut[curFontMapOut];
                        pInfo->pMap = mapOut;
                        curFontMapOut++;
                        if(curFontMapOut > NNS_FONT_MAP_OUT_MAX) {
                            curFontMapOut = 0;
                        }
                        while(curMap != NULL) {
                            WIN_NNSG2dFontCodeMap * winMap = (WIN_NNSG2dFontCodeMap*)curMap;
                            mapOut->ccodeBegin = winMap->ccodeBegin;
                            mapOut->ccodeEnd = winMap->ccodeEnd;
                            mapOut->mappingMethod = winMap->mappingMethod;
                            mapOut->reserved = winMap->reserved;
                            mapOut->mapInfo = winMap->mapInfo;
                            if(winMap->pNext != 0) {
                                mapOut->pNext = &fontMapOut[curFontMapOut];
                                mapOut = &fontMapOut[curFontMapOut];
                                curFontMapOut++;
                                if(curFontMapOut > NNS_FONT_MAP_OUT_MAX) {
                                    curFontMapOut = 0;
                                }
                                curMap = (void*)((u64)pHeader + (u64)winMap->pNext);
                            } else {
                                curMap = NULL;
                                mapOut->pNext = NULL;
                            }
                        }
                        #endif
                        NNS_G2D_POINTER_ASSERT(pInfo->pMap)
                    }
                }
                break;
            case NNS_G2D_BINBLK_SIG_CGLPDATA:
            {
                NNSG2dFontGlyph * pGlyph = (NNSG2dFontGlyph *)((u8 *)pBlock + sizeof(*pBlock));
                NNS_G2D_MIN_ASSERT(pGlyph->cellWidth, 1);
                NNS_G2D_MIN_ASSERT(pGlyph->cellHeight, 1);
                NNS_G2D_MIN_ASSERT(pGlyph->cellSize, 1);
                NNS_G2D_MINMAX_ASSERT(pGlyph->bpp, 1, 8);
            }
            break;
            case NNS_G2D_BINBLK_SIG_CWDHDATA:
            {
                #ifndef SDK_PORT
                NNSG2dFontWidth * pWidth = (NNSG2dFontWidth *)((u8 *)pBlock + sizeof(*pBlock));

                NNS_G2D_ASSERT(pWidth->indexBegin <= pWidth->indexEnd);

                if (pWidth->pNext != NULL) {
                    ResolveOffset((void **)&(pWidth->pNext), pHeader);
                    NNS_G2D_POINTER_ASSERT(pWidth->pNext)
                }
                #endif
            }
            break;
            case NNS_G2D_BINBLK_SIG_CMAPDATA:
            {
                #ifndef SDK_PORT
                NNSG2dFontCodeMap * pMap = (NNSG2dFontCodeMap *)((u8 *)pBlock + sizeof(*pBlock));

                NNS_G2D_ASSERT(pMap->ccodeBegin <= pMap->ccodeEnd);
                NNS_G2D_ASSERT((pMap->mappingMethod == NNS_G2D_MAPMETHOD_DIRECT)
                               || (pMap->mappingMethod == NNS_G2D_MAPMETHOD_TABLE)
                               || (pMap->mappingMethod == NNS_G2D_MAPMETHOD_SCAN));

                if (pMap->pNext != NULL) {
                    ResolveOffset((void **)&(pMap->pNext), pHeader);
                    NNS_G2D_POINTER_ASSERT(pMap->pNext)
                }
                #endif
            }
            break;
            default:
                NNS_G2D_ASSERTMSG(FALSE, "The font has unknown block.");
                break;
            }

            pBlock = (NNSG2dBinaryBlockHeader *)((u8 *)pBlock + pBlock->size);
            nBlocks++;
        }
    }

    NNS_G2D_ASSERTMSG(pInfo != NULL, "The font has no Information Block.");
    NNS_G2D_ASSERTMSG(pInfo->pGlyph != NULL, "The font has no Glyph Block.");

    NNS_G2D_ASSERT(pInfo->alterCharIndex < NNS_G2D_FONT_MAX_GLYPH_INDEX((NNSG2dFont *)&pInfo));

    #ifdef SDK_PORT
    return pInfo;
    #endif
}

#ifndef SDK_FINALROM
static void PrintFontInformation (const NNSG2dFontInformation * pInfo)
{
    const static char * fontTypeString[] = {
        "NNS_G2D_FONTTYPE_GLYPH",
        "NNS_G2D_FONTTYPE_SHEET"
    };
    const static char * encodingString[] = {
        "NNS_G2D_FONT_ENCODING_UTF8",
        "NNS_G2D_FONT_ENCODING_UTF16",
        "NNS_G2D_FONT_ENCODING_SJIS",
        "NNS_G2D_FONT_ENCODING_CP1252",
    };

    NNS_G2D_POINTER_ASSERT(pInfo);
    NNS_G2D_MINMAX_ASSERT(pInfo->fontType, 0, NNS_G2D_NUM_OF_FONTTYPE - 1);
    NNS_G2D_MINMAX_ASSERT(pInfo->encoding, 0, NNS_G2D_NUM_OF_ENCODING - 1);

    OS_Printf(" NNSG2dFontInformation\n");
    OS_Printf(" fontType                = %s\n", fontTypeString[pInfo->fontType]);
    OS_Printf(" linefeed                = %d px\n", pInfo->linefeed);
    OS_Printf(" alterCharIndex          = 0x%04X\n", pInfo->alterCharIndex);
    OS_Printf(" defaultWidth.left       = %d px\n", pInfo->defaultWidth.left);
    OS_Printf(" defaultWidth.glyphWidth = %d px\n", pInfo->defaultWidth.glyphWidth);
    OS_Printf(" defaultWidth.charWidth  = %d px\n", pInfo->defaultWidth.charWidth);
    OS_Printf(" encoding                = %s\n", encodingString[pInfo->encoding]);
}

static void PrintFontGlyph (const NNSG2dFontGlyph * pGlyph)
{
    NNS_G2D_POINTER_ASSERT(pGlyph);

    OS_Printf(" NNSG2dFontGlyph\n");
    OS_Printf(" cellWidth          = %d px\n", pGlyph->cellWidth);
    OS_Printf(" cellHeight         = %d px\n", pGlyph->cellHeight);
    OS_Printf(" cellSize           = %d byte\n", pGlyph->cellSize);
    OS_Printf(" baselinePos        = %d px\n", pGlyph->baselinePos);
    OS_Printf(" maxCharWidth       = %d px\n", pGlyph->maxCharWidth);
    OS_Printf(" bpp                = %d bpp\n", pGlyph->bpp);
    OS_Printf(" flags              = ");

    if ((pGlyph->flags & NNS_G2D_FONT_FLAG_TBRL) != 0) {
        OS_Printf("NNS_G2D_FONT_FLAG_TBRL ");
    }

    switch (pGlyph->flags & NNS_G2D_FONT_FLAG_ROT_MASK) {
    case NNS_G2D_FONT_FLAG_ROT_0:
        OS_Printf("NNS_G2D_FONT_FLAG_ROT_0\n");
        break;
    case NNS_G2D_FONT_FLAG_ROT_90:
        OS_Printf("NNS_G2D_FONT_FLAG_ROT_90\n");
        break;
    case NNS_G2D_FONT_FLAG_ROT_180:
        OS_Printf("NNS_G2D_FONT_FLAG_ROT_180\n");
        break;
    case NNS_G2D_FONT_FLAG_ROT_270:
        OS_Printf("NNS_G2D_FONT_FLAG_ROT_270\n");
        break;
    }
}

static void PrintFontWidth (const NNSG2dFontWidth * pWidth)
{
    NNS_G2D_POINTER_ASSERT(pWidth);

    OS_Printf(" NNSG2dFontWidth\n");
    OS_Printf(" indexBegin         = 0x%04X\n", pWidth->indexBegin);
    OS_Printf(" indexEnd           = 0x%04X\n", pWidth->indexEnd);
}

static void PrintFontCMapTable (const u16 * table, int num, int ccodeOffset)
{
    int i;

    NNS_G2D_POINTER_ASSERT(table);

    for (i = 0; i < num; i++) {
        OS_Printf("   %4d: 0x%04X -> %d\n", i,
                  i + ccodeOffset, table[i]);
    }
}

static void PrintFontCMapScanList (const NNSG2dCMapInfoScan * scan)
{
    int i;

    NNS_G2D_POINTER_ASSERT(scan);

    for (i = 0; i < scan->num; i++) {
        OS_Printf("   %4d: 0x%04X -> %d\n", i,
                  scan->entries[i].ccode, scan->entries[i].index);
    }
}

static void PrintFontCMap (const NNSG2dFontCodeMap * pCMap)
{
    const static char * mappingMethodString[] = {
        "NNS_G2D_MAPMETHOD_DIRECT",
        "NNS_G2D_MAPMETHOD_TABLE",
        "NNS_G2D_MAPMETHOD_SCAN",
        "NNS_G2D_NUM_OF_MAPMETHOD"
    };

    NNS_G2D_POINTER_ASSERT(pCMap);
    NNS_G2D_MINMAX_ASSERT(pCMap->mappingMethod, 0, NNS_G2D_NUM_OF_MAPMETHOD - 1);

    OS_Printf(" NNSG2dFontCodeMap\n");
    OS_Printf(" ccodeBegin         = 0x%04X\n", pCMap->ccodeBegin);
    OS_Printf(" ccodeEnd           = 0x%04X\n", pCMap->ccodeEnd);
    OS_Printf(" mappingMethod      = %s\n", mappingMethodString[pCMap->mappingMethod]);

    switch (pCMap->mappingMethod) {
    case NNS_G2D_MAPMETHOD_DIRECT:
    {
        OS_Printf(" indexOffset        = 0x%04X\n", pCMap->mapInfo[0]);
    }
    break;
    case NNS_G2D_MAPMETHOD_TABLE:
    {
#ifdef NNS_G2D_FONT_SHOW_CMAP_LIST
        int ccodeOffset = pCMap->ccodeBegin;
        int num = pCMap->ccodeEnd - pCMap->ccodeBegin + 1;

        PrintFontCMapTable(pCMap->mapInfo, num, ccodeOffset);
#endif
    }
    break;
    case NNS_G2D_MAPMETHOD_SCAN:
    {
        const NNSG2dCMapInfoScan * const ws = (NNSG2dCMapInfoScan *)(pCMap->mapInfo);
        OS_Printf(" num                = %d\n", ws->num);
#ifdef NNS_G2D_FONT_SHOW_CMAP_LIST
        PrintFontCMapScanList(ws);
#endif
    }
    break;
    default:
        OS_Panic("unknwon MAPMETHOD");
    }
}

static NNS_G2D_INLINE void PrintSpliter (void)
{
    OS_Printf("---------------------------------------------\n");
}

void NNS_G2dPrintFont (const NNSG2dFont * pFont)
{
    NNS_G2D_POINTER_ASSERT(pFont);

    {
        const NNSG2dFontWidth * pWidth = pFont->pRes->pWidth;
        const NNSG2dFontCodeMap * pCMap = pFont->pRes->pMap;

        PrintSpliter();
        PrintFontInformation(pFont->pRes);

        PrintSpliter();
        PrintFontGlyph(pFont->pRes->pGlyph);

        while (pWidth != NULL) {
            PrintSpliter();
            PrintFontWidth(pWidth);
            pWidth = pWidth->pNext;
        }

        while (pCMap != NULL) {
            PrintSpliter();
            PrintFontCMap(pCMap);
            pCMap = pCMap->pNext;
        }

        PrintSpliter();
    }
}
#endif
