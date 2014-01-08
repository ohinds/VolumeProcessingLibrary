/*
 * io.h --
 *
 */
/*
 * $Id: io.h,v 1.1 2005/07/28 19:25:50 oph Exp $
 */
#ifndef _IO
#define _IO

#include "jpeg.h"

/*
 * Size of the input and output buffer
 */
#define JPEG_BUF_SIZE   4096 

/*
 * The following variables keep track of the input and output
 * buffer for the JPEG data.
 */
extern char   outputBuffer[JPEG_BUF_SIZE];      /* output buffer              */
extern int    numOutputBytes;                   /* bytes in the output buffer */
extern Uchar *inputBuffer;                      /* Input buffer for JPEG data */
extern int    inputBufferOffset;                /* Offset of current byte     */

#endif /* _IO */
