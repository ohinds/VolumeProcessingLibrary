/*
 * JPEGLess.h
 *
 * ---------------------------------------------------------------
 *
 * Lossless JPEG compression and decompression algorithms.
 *
 * ---------------------------------------------------------------
 *
 * It is based on the program originally named ljpgtopnm and pnmtoljpg.
 * Major portions taken from the Independetn JPEG Group' software, and
 * from the Cornell lossless JPEG code (the original copyright notices
 * for those packages appears below).
 *
 * ---------------------------------------------------------------
 *
 * This is the main routine for the lossless JPEG decoder.  Large
 * parts are stolen from the IJG code
 */
/*
 * $Id: jpegless.h,v 1.1 2005/07/28 19:25:50 oph Exp $
 */
#include "jpeg.h"

#ifndef _JPEGLOSSLESS_
#define _JPEGLOSSLESS_

#if defined(__cplusplus)
extern "C"
{
#endif



/* Global variables for lossless encoding process */

int psvSet[7];        /* the PSV (prediction selection value) set    */
int numSelValue;      /* number of PSVs in psvSet                    */
long inputFileBytes;  /* the input file size in bytes                */
long outputFileBytes; /* the output file size in bytes               */
long totalHuffSym[7]; /* total bits of category symbols for each PSV */
long totalAddBits[7]; /* total bits of additional bits for each PSV  */
int  verbose;         /* the verbose flag                            */


/*
 * read a JPEG lossless (8 or 16 bit) image in a file and decode it
 */
short JPEGLosslessDecodeImage (StreamIN, unsigned short *, int , int);


#if defined(__cplusplus)
}
#endif

#endif /* _JPEGLOSSLESS_ */

