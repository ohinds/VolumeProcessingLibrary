/*
 * proto.h --
 *
 * Part of the Independent JPEG Group's software.
 * See the file Copyright for more details.
 */
/*
 * $Id: proto.h,v 1.1 2005/07/28 19:25:50 oph Exp $
 */
#ifndef _PROTO
#define _PROTO

#ifdef __STDC__
        # define P(s) s
#else
        # define P(s) ()
#endif

#include "mcu.h"


/* huffd.c */
void  HuffDecoderInit P((DecompressInfo *dcPtr ));
void  DecodeImage P((DecompressInfo *dcPtr, unsigned short **image, int depth));
void  FixHuffTbl (HuffmanTable *htbl);
void  PmPutRow24(MCU *RowBuf, int numCol, unsigned char **image);
void  PmPutRow16(MCU *RowBuf, int numCol, unsigned short **image);
void  PmPutRow8(MCU *RowBuf, int numCol, unsigned char **image);
void  DecodeFirstRow (DecompressInfo *dcPtr, MCU *curRowBuf);

/* decomp.c */
int   ReadJpegData P((Uchar *buffer , int numBytes));
short JPEGLosslessDecodeImage (StreamIN inFile, unsigned short *image16, int depth, int length);

/* read.c */
void  ReadFileHeader P((DecompressInfo *dcPtr ));
int   ReadScanHeader P((DecompressInfo *dcPtr ));
int   GetJpegChar(void);
void  UnGetJpegChar(int ch);  

/* util.c */
int   JroundUp P((int a , int b ));
void  DecoderStructInit P((DecompressInfo *dcPtr ));

 /* mcu.c */
void  InitMcuTable P((int numMCU , int blocksInMCU ));
void  FreeMcuTable(void);
void  PrintMCU P((int blocksInMCU , MCU mcu ));

#undef P
#endif /* _PROTO */

