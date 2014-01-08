/*
 * huffd.c --
 *
 * Code for JPEG lossless decoding.  Large parts are grabbed from the IJG
 * software
 */
/*
 * $Id: huffd.c,v 1.1 2005/07/28 19:25:50 oph Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpeg.h"
#include "mcu.h"
#include "io.h"
#include "proto.h"
#include "predict.h"


#define RST0    0xD0                /* RST0 marker code */

static long getBuffer;              /* current bit-extraction buffer */
static int  bitsLeft;               /* # of unused bits in it */

/*
 * The following variables keep track of the input buffer
 * for the JPEG data, which is read by ReadJpegData.
 */
Uchar *inputBuffer = 0;           /* Input buffer for JPEG data */
int inputBufferOffset = 0;        /* Offset of current byte */

/*
 * Code for extracting the next N bits from the input stream.
 * (N never exceeds 15 for JPEG data.)
 * This needs to go as fast as possible!
 *
 * We read source bytes into getBuffer and dole out bits as needed.
 * If getBuffer already contains enough bits, they are fetched in-line
 * by the macros get_bits() and get_bit().  When there aren't enough bits,
 * FillBitBuffer is called; it will attempt to fill getBuffer to the
 * "high water mark", then extract the desired number of bits.  The idea,
 * of course, is to minimize the function-call overhead cost of entering
 * FillBitBuffer.
 * On most machines MIN_GET_BITS should be 25 to allow the full 32-bit width
 * of getBuffer to be used.  (On machines with wider words, an even larger
 * buffer could be used.)  
 */

#define BITS_PER_LONG (8*sizeof(long))
#define MIN_GET_BITS  (BITS_PER_LONG-7) /* max value for long getBuffer */

/*
 * bmask[n] is mask for n rightmost bits
 */
static int bmask[] = {0x0000,
         0x0001, 0x0003, 0x0007, 0x000F,
         0x001F, 0x003F, 0x007F, 0x00FF,
         0x01FF, 0x03FF, 0x07FF, 0x0FFF,
         0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF};

/*
 *--------------------------------------------------------------
 *
 * FillBitBuffer --
 *
 *        Load up the bit buffer with at least nbits
 *        Process any stuffed bytes at this time.
 *
 * Results:
 *        None
 *
 * Side effects:
 *        The bitwise global variables are updated.
 *
 *--------------------------------------------------------------
 */
#define FillBitBuffer(nbits) {                                          \
    int c, c2;                                                          \
    while (bitsLeft < MIN_GET_BITS) {                                   \
        c = GetJpegChar ();                                             \
        /* If it's 0xFF, check and discard stuffed zero byte */         \
        if (c == 0xFF) {                                                \
            c2 = GetJpegChar ();                                        \
            if (c2 != 0) {                                              \
                UnGetJpegChar (c2);                                     \
                UnGetJpegChar (c);                                      \
                c = 0;                                                  \
            }                                                           \
        }/*endif 0xFF*/                                                 \
        /* OK, load c into getBuffer */                                 \
        getBuffer = (getBuffer << 8) | c;                               \
        bitsLeft += 8;                                                  \
    }/*endwhile*/                                                       \
}/*endof FillBitBuffer*/

/* Macros to make things go at some speed! */
/* NB: parameter to get_bits should be simple variable, not expression */

#define show_bits(nbits,rv) {                                           \
    if (bitsLeft < nbits) FillBitBuffer(nbits);                         \
    rv = (getBuffer >> (bitsLeft-(nbits))) & bmask[nbits];              \
}

#define show_bits8(rv) {                                                \
        if (bitsLeft < 8) FillBitBuffer(8);                             \
        rv = (getBuffer >> (bitsLeft-8)) & 0xff;                        \
}

#define flush_bits(nbits) {                                             \
        bitsLeft -= (nbits);                                            \
}

#define get_bits(nbits,rv) {                                            \
        if (bitsLeft < nbits) FillBitBuffer(nbits);                     \
        rv = ((getBuffer >> (bitsLeft -= (nbits)))) & bmask[nbits];     \
}

#define get_bit(rv) {                                                   \
        if (!bitsLeft) FillBitBuffer(1);                                \
        rv = (getBuffer >> (--bitsLeft)) & 1;                           \
}

/*
 *--------------------------------------------------------------
 *
 * PmPutRow --
 *
 *      Output one row of pixels stored in RowBuf.
 *
 * Results:
 *      None
 *
 * Side effects:
 *      One row of pixels are write to file pointed by outFile.
 *
 *--------------------------------------------------------------
 */   


void PmPutRow24(MCU *RowBuf, int numCol, unsigned char **image)
{
        register int col;

        for (col = 0; col < numCol; col++)
    {
             /* take each RGB column */
             **image = (unsigned char) RowBuf[col][0];
             *(*image+1) = (unsigned char) RowBuf[col][1];
             *(*image+2) = (unsigned char) RowBuf[col][2];
             (*image)+=3;
    }
}

                                   
void PmPutRow16(MCU *RowBuf, int numCol, unsigned short **image)
{  
        register int col;          

        for (col = 0; col < numCol; col++)
    {                   
             **image = (unsigned short) RowBuf[col][0];
             (*image)++;                                        
    }
}   


void PmPutRow8(MCU *RowBuf, int numCol, unsigned char **image)
{  
        register int col;          

        for (col = 0; col < numCol; col++)
    {                   
             **image = (unsigned char) RowBuf[col][0];
             (*image)++;                                        
    }   
}   
                                                                           



/*
 *--------------------------------------------------------------
 *
 * HuffDecode --
 *
 *        Taken from Figure F.16: extract next coded symbol from
 *        input stream.  This should becode a macro.
 *
 * Results:
 *        Next coded symbol
 *
 * Side effects:
 *        Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
#define HuffDecode(htbl,rv)                                             \
{                                                                       \
    int l, code, temp;                                                  \
                                                                        \
    /*                                                                  \
     * If the huffman code is less than 8 bits, we can use the fast     \
     * table lookup to get its value.  It's more than 8 bits about      \
     * 3-4% of the time.                                                \
     */                                                                 \
    show_bits8(code);                                                   \
    if (htbl->numbits[code]) {                                          \
        flush_bits(htbl->numbits[code]);                                \
        rv=htbl->value[code];                                           \
    }                                                                   \
    else {                                                              \
        flush_bits(8);                                                  \
        l = 8;                                                          \
        while (code > htbl->maxcode[l]) {                               \
            get_bit(temp);                                              \
            code = (code << 1) | temp;                                  \
            l++;                                                        \
        }                                                               \
                                                                        \
        /*                                                              \
         * With garbage input we may reach the sentinel value l = 17.   \
         */                                                             \
                                                                        \
        if (l > 16) {                                                   \
            fprintf (stderr, "Corrupt JPEG data: bad Huffman code\n");  \
            rv = 0; /* fake a zero as the safest result */              \
        } else {                                                        \
            rv = htbl->huffval[htbl->valptr[l] +                        \
                ((int)(code - htbl->mincode[l]))];                      \
        }                                                               \
    }/*endelse*/                                                        \
}/*HuffDecode*/

/*
 *--------------------------------------------------------------
 *
 * HuffExtend --
 *
 *        Code and table for Figure F.12: extend sign bit
 *
 * Results:
 *        The extended value.
 *
 * Side effects:
 *        None.
 *
 *--------------------------------------------------------------
 */
static int extendTest[16] =        /* entry n is 2**(n-1) */
{0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
 0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000};

static int extendOffset[16] =        /* entry n is (-1 << n) + 1 */
{0, ((-1) << 1) + 1, ((-1) << 2) + 1, ((-1) << 3) + 1, ((-1) << 4) + 1,
 ((-1) << 5) + 1, ((-1) << 6) + 1, ((-1) << 7) + 1, ((-1) << 8) + 1,
 ((-1) << 9) + 1, ((-1) << 10) + 1, ((-1) << 11) + 1, ((-1) << 12) + 1,
 ((-1) << 13) + 1, ((-1) << 14) + 1, ((-1) << 15) + 1};

#define HuffExtend(x,s) {                                               \
    if ((x) < extendTest[s]) {                                          \
        (x) += extendOffset[s];                                         \
    }                                                                   \
}

/*
 *--------------------------------------------------------------
 *
 * HuffDecoderInit --
 *
 *        Initialize for a Huffman-compressed scan.
 *        This is invoked after reading the SOS marker.
 *
 * Results:
 *        None
 *
 * Side effects:
 *        None.
 *
 *--------------------------------------------------------------
 */
void HuffDecoderInit (DecompressInfo *dcPtr)
{
    short ci;
    JpegComponentInfo *compptr;

    /*
     * Initialize static variables
     */
    bitsLeft = 0;

    for (ci = 0; ci < dcPtr->compsInScan; ci++) {
        compptr = dcPtr->curCompInfo[ci];
        /*
         * Make sure requested tables are present
         */
        if (dcPtr->dcHuffTblPtrs[compptr->dcTblNo] == NULL) { 
            fprintf (stderr, "Error: Use of undefined Huffman table\n");
            /* exit (1); */
            dcPtr->error = -1; return;
        }

        /*
         * Compute derived values for Huffman tables.
         * We may do this more than once for same table, but it's not a
         * big deal
         */
        FixHuffTbl (dcPtr->dcHuffTblPtrs[compptr->dcTblNo]);
    }

    /*
     * Initialize restart stuff
     */
    dcPtr->restartInRows = (dcPtr->restartInterval)/(dcPtr->imageWidth);
    dcPtr->restartRowsToGo = dcPtr->restartInRows;
    dcPtr->nextRestartNum = 0;
}

/*
 *--------------------------------------------------------------
 *
 * ProcessRestart --
 *
 *        Check for a restart marker & resynchronize decoder.
 *
 * Results:
 *        None.
 *
 * Side effects:
 *        BitStream is parsed, bit buffer is reset, etc.
 *
 *--------------------------------------------------------------
 */
static void ProcessRestart(DecompressInfo *dcPtr)
{
    int c, nbytes;
    /*short ci;*/

    /*
     * Throw away any unused bits remaining in bit buffer
     */
    nbytes = bitsLeft / 8;
    bitsLeft = 0;

    /*
     * Scan for next JPEG marker
     */
    do 
        {
                do 
                {   /* skip any non-FF bytes */
                    nbytes++;
                    c = GetJpegChar();
                } 
                while (c != 0xFF);
        
                do 
                {   /* skip any duplicate FFs */
                    /*
                     * we don't increment nbytes here since extra FFs are legal
                     */
                    c = GetJpegChar ();
                } 
                while (c == 0xFF);
    } 
        while (c == 0); /* repeat if it was a stuffed FF/00 */

    if (c != (RST0 + dcPtr->nextRestartNum)) {

        /*
         * Uh-oh, the restart markers have been messed up too.
         * Just bail out.
         */
        fprintf (stderr, "Error: Corrupt JPEG data.  Exiting...\n");
        /* exit(-1); */ 
        dcPtr->error = -1; return;
    }

    /*
     * Update restart state
     */
    dcPtr->restartRowsToGo = dcPtr->restartInRows;
    dcPtr->nextRestartNum = (dcPtr->nextRestartNum + 1) & 7;
}

/*
 *--------------------------------------------------------------
 *
 * DecodeFirstRow --
 *
 *        Decode the first raster line of samples at the start of 
 *      the scan and at the beginning of each restart interval.
 *        This includes modifying the component value so the real
 *      value, not the difference is returned.
 *
 * Results:
 *        None.
 *
 * Side effects:
 *        Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
void DecodeFirstRow (DecompressInfo *dcPtr, MCU *curRowBuf)
{
    register short curComp,ci;
    register int s,col,compsInScan,numCOL;
    register JpegComponentInfo *compptr;
    int Pr,Pt,d;
    HuffmanTable *dctbl;

    Pr=dcPtr->dataPrecision;
    Pt=dcPtr->Pt;
    compsInScan=dcPtr->compsInScan;
    numCOL=dcPtr->imageWidth;

    /*
     * the start of the scan or at the beginning of restart interval.
     */
    for (curComp = 0; curComp < compsInScan; curComp++) {
        ci = dcPtr->MCUmembership[curComp];
        compptr = dcPtr->curCompInfo[ci];
        dctbl = dcPtr->dcHuffTblPtrs[compptr->dcTblNo];

        /*
         * Section F.2.2.1: decode the difference
         */
        HuffDecode (dctbl,s);
        if (s) {
           if (s == 16) {      /* special case: always output 32768 */
              d = 32768;
           } else {            /* normal  case: fetch subsequent bits */
              get_bits(s,d);
              HuffExtend(d,s);
           }
        } else {
           d = 0;
        }

        /* 
         * Add the predictor to the difference.
         */
        curRowBuf[0][curComp]=d+(1<<(Pr-Pt-1));
    }

    /*
     * the rest of the first row
     */
    for (col=1; col<numCOL; col++) {
        for (curComp = 0; curComp < compsInScan; curComp++) {
            ci = dcPtr->MCUmembership[curComp];
            compptr = dcPtr->curCompInfo[ci];
            dctbl = dcPtr->dcHuffTblPtrs[compptr->dcTblNo];

            /*
             * Section F.2.2.1: decode the difference
             */
            HuffDecode (dctbl,s);
            if (s) {
               if (s == 16) {      /* special case: always output 32768 */
                  d = 32768;
               } else {            /* normal  case: fetch subsequent bits */
                  get_bits(s,d);
                  HuffExtend(d,s);
               }
            } else {
               d = 0;
            }

            /* 
             * Add the predictor to the difference.
             */
            curRowBuf[col][curComp]=d+curRowBuf[col-1][curComp];
        }
    }

    if (dcPtr->restartInRows) {
       (dcPtr->restartRowsToGo)--;
    }
}/*endof DecodeFirstRow*/

/*
 *--------------------------------------------------------------
 *
 * DecodeImage --
 *
 *      Decode the input stream. This includes modifying
 *      the component value so the real value, not the
 *      difference is returned.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
void DecodeImage (DecompressInfo *dcPtr, unsigned short **image, int depth)
{
    register int s, d, col, row;
    register short curComp, ci;
    HuffmanTable *dctbl;
    JpegComponentInfo *compptr;
    int predictor;
    int numCOL, numROW, compsInScan;
    MCU *prevRowBuf, *curRowBuf;
    int imagewidth, Pt, psv;
    unsigned short *image16tmp;
    unsigned char *image8tmp, *image24tmp;
             
    numCOL      = imagewidth=dcPtr->imageWidth;
    numROW      = dcPtr->imageHeight;
    compsInScan = dcPtr->compsInScan;
    Pt          = dcPtr->Pt;
    psv         = dcPtr->Ss;
    prevRowBuf  = mcuROW2;
    curRowBuf   = mcuROW1;
  
    if (depth == 8) 
                image8tmp  = (unsigned char  *) *image;
    else if (depth == 16)            
                image16tmp = (unsigned short *) *image;    
    else if (depth == 24)
                image24tmp = (unsigned char  *) *image;
    else {
                fprintf(stderr,"Unsupported image depth %d\n",depth);
                dcPtr->error = -1; return;
         } 

    /*
     * Decode the first row of image. Output the row and
     * turn this row into a previous row for later predictor
     * calculation.
     */         
    row = 0;
    DecodeFirstRow (dcPtr, curRowBuf);
    if (depth == 8) 
                PmPutRow8  (curRowBuf, numCOL, &image8tmp);
    else if (depth == 16)
                PmPutRow16 (curRowBuf, numCOL, &image16tmp);
    else if (depth == 24)
                PmPutRow24 (curRowBuf, numCOL, &image24tmp);

    swap(MCU *, prevRowBuf, curRowBuf);

    /* optimal case : 8 bit image, one color component, no restartInRows */
    if ((depth == 8) && (compsInScan == 1) && (dcPtr->restartInRows == 0)) 
        {

      unsigned char        *curPixelPtr;
      int                 left,upper,diag;
      
      /* initializations */
      curComp      = 0;
      ci           = dcPtr->MCUmembership[curComp];
      compptr      = dcPtr->curCompInfo[ci];
      dctbl        = dcPtr->dcHuffTblPtrs[compptr->dcTblNo];
      curPixelPtr  = image8tmp;

      for (row=1; row<numROW; row++) 
          {

          /* Upper neighbor is predictor for the first column */
          /* Section F.2.2.1: decode the difference */
          HuffDecode (dctbl,s);
          if (s) {
             if (s == 16) {      /* special case: always output 32768 */
                d = 32768;
             } else {            /* normal  case: fetch subsequent bits */
                get_bits(s,d);
                HuffExtend(d,s);
             }
          } else {
             d = 0;
          }

          *curPixelPtr = (unsigned char) (d + *(curPixelPtr - numCOL));
          curPixelPtr++;

          if (psv == 1) {
            for (col=1; col < numCOL; col++) {
              /* Section F.2.2.1: decode the difference */
              HuffDecode (dctbl, s);
              if (s) {
                 if (s == 16) {      /* special case: always output 32768 */
                    d = 32768;
                 } else {            /* normal  case: fetch subsequent bits */
                    get_bits(s,d);
                    HuffExtend(d,s);
                 }
              } else {
                 d = 0;
              }

              *curPixelPtr = (unsigned char) (d + *(curPixelPtr - 1));
              curPixelPtr++;
            }/*endfor col*/
          }/*endif*/
          
          else {
            for (col=1; col < numCOL; col++) {
              /* Section F.2.2.1: decode the difference */
              HuffDecode (dctbl, s);
              if (s) {
                 if (s == 16) {      /* special case: always output 32768 */
                    d = 32768;
                 } else {            /* normal  case: fetch subsequent bits */
                    get_bits(s,d);
                    HuffExtend(d,s);
                 }
              } else {
                 d = 0;
              }

              /* Predict : All predictors are calculated according to psv */
              switch (psv) {
                case 0: predictor = 0;                               break;
                case 2: predictor = *(curPixelPtr - numCOL);         break;
                case 3: predictor = *(curPixelPtr - numCOL - 1);     break;
                case 4: upper     = *(curPixelPtr - numCOL);
                            left      = *(curPixelPtr - 1);
                            diag      = *(curPixelPtr - numCOL - 1);
                            predictor = left + upper - diag;         break;
                case 5: upper     = *(curPixelPtr - numCOL);
                        left      = *(curPixelPtr - 1);
                        diag      = *(curPixelPtr - numCOL - 1);
                        predictor = left+((upper-diag)>>1);          break;
                case 6: upper     = *(curPixelPtr - numCOL);
                            left      = *(curPixelPtr - 1);
                            diag      = *(curPixelPtr - numCOL - 1);
                            predictor = upper+((left-diag)>>1);      break;
                case 7: upper     = *(curPixelPtr - numCOL);
                            left      = *(curPixelPtr - 1);
                        predictor = (left+upper)>>1;                 break;
                default : predictor = 0;
              }/*endsandwich*/

              *curPixelPtr = (unsigned char) (d + predictor);
              curPixelPtr++;
            }/*endfor col*/
          }/*endelse*/
          
      }/*endfor row*/
    }/*endif fast case*/
    
    else { /*normal case with 16 bits or color or ...*/
      for (row=1; row<numROW; row++) {

        /*
         * Account for restart interval, process restart marker if needed.
         */
        if (dcPtr->restartInRows) 
                {
           if (dcPtr->restartRowsToGo == 0) 
                   {
              ProcessRestart (dcPtr); if (dcPtr->error) return;
            
              /*
               * Reset predictors at restart.
               */
              DecodeFirstRow(dcPtr,curRowBuf);       
              if (depth == 8) 
                        PmPutRow8  (curRowBuf, numCOL, &image8tmp);
              else if (depth == 16)
                        PmPutRow16 (curRowBuf, numCOL, &image16tmp);
              else if (depth == 24)
                        PmPutRow24 (curRowBuf, numCOL, &image24tmp);

              swap(MCU *,prevRowBuf,curRowBuf);
              continue;
           }
           dcPtr->restartRowsToGo--;
        }/*endif*/

        /*
         * For the rest of the column on this row, predictor
         * calculations are base on PSV. 
         */
        
        /* several color components to decode (RGB colors)*/

        /* The upper neighbors are predictors for the first column. */
        for (curComp = 0; curComp < compsInScan; curComp++) 
                {
            ci = dcPtr->MCUmembership[curComp];
            compptr = dcPtr->curCompInfo[ci];
            dctbl = dcPtr->dcHuffTblPtrs[compptr->dcTblNo];

            /* Section F.2.2.1: decode the difference */
            HuffDecode (dctbl,s);
            if (s) {
               if (s == 16) {      /* special case: always output 32768 */
                  d = 32768;
               } else {            /* normal  case: fetch subsequent bits */
                  get_bits(s,d);
                  HuffExtend(d,s);
               }
            } else {
               d = 0;
            }

            curRowBuf[0][curComp]=d+prevRowBuf[0][curComp];
        }/*endfor curComp*/
          
        for (col=1; col < numCOL; col++) 
                {
            for (curComp = 0; curComp < compsInScan; curComp++) 
                        {
                ci = dcPtr->MCUmembership[curComp];
                compptr = dcPtr->curCompInfo[ci];
                dctbl = dcPtr->dcHuffTblPtrs[compptr->dcTblNo];

                /* Section F.2.2.1: decode the difference */
                HuffDecode (dctbl, s);
                if (s) {
                   if (s == 16) {      /* special case: always output 32768 */
                      d = 32768;
                   } else {            /* normal  case: fetch subsequent bits */
                      get_bits(s,d);
                      HuffExtend(d,s);
                   }
                } else {
                   d = 0;
                }

                QuickPredict (col,curComp,curRowBuf,prevRowBuf,psv,&predictor);

                curRowBuf[col][curComp]=d+predictor;
            }/*endfor curComp*/
        }/*endfor col*/
        
              if (depth == 8) 
                        PmPutRow8  (curRowBuf, numCOL, &image8tmp);
              else if (depth == 16)
                        PmPutRow16 (curRowBuf, numCOL, &image16tmp);
              else if (depth == 24)
                        PmPutRow24 (curRowBuf, numCOL, &image24tmp);

              swap(MCU *, prevRowBuf, curRowBuf);
      
      }/*endfor row*/
    }/*endelse*/
}/*endofmethod DecodeImage*/
