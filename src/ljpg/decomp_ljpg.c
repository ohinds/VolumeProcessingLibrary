/*
 * decomp.c --
 *
 * This is the routine that is called to decompress a frame 
 * image data. It is based on the program originally named ljpgtopnm.c.
 * Major portions taken from the Independent JPEG Group' software, and
 * from the Cornell lossless JPEG code
 */
/*
 * $Id: decomp_ljpg.c,v 1.2 2005/08/25 18:28:31 oph Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include "io.h"
#include "jpeg.h"
#include "mcu.h"
#include "proto.h"

static DecompressInfo  dcInfo;
static StreamIN        JpegInFile;     

/*
 *--------------------------------------------------------------
 *
 * ReadJpegData --
 *
 *        This is an interface routine to the JPEG library.  The
 *        JPEG library calls this routine to "get more data"
 *
 * Results:
 *        Number of bytes actually returned.
 *
 * Side effects:
 *        None.
 *
 *--------------------------------------------------------------
 */
static void efree(unsigned char **ptr)
{
        if((*ptr) != 0)
                free((*ptr));
        *ptr = 0;
}


int ReadJpegData (Uchar *buffer, int numBytes)
{
    unsigned long size = sizeof(unsigned char);

    fread(buffer,size,(unsigned)numBytes,JpegInFile);

    return numBytes;  
}


short JPEGLosslessDecodeImage (StreamIN inFile, unsigned short *image16, int depth, int length)
{ 
    /* Initialization */
    JpegInFile = inFile;
    MEMSET (&dcInfo, 0, sizeof (dcInfo));
    inputBufferOffset = 0;
    
    /* Allocate input buffer */
    inputBuffer = (unsigned char*)malloc((size_t)length+5);
    if (inputBuffer == NULL)
                return -1;

        /* Read input buffer */
    ReadJpegData (inputBuffer, length);
    inputBuffer [length] = (unsigned char)EOF;
    
        /* Read JPEG File header */
    ReadFileHeader (&dcInfo);
    if (dcInfo.error) { efree (&inputBuffer); return -1; }

    /* Read the scan header */
    if (!ReadScanHeader (&dcInfo)) { efree (&inputBuffer); return -1; }
    
    /* 
     * Decode the image bits stream. Clean up everything when
     * finished decoding.
     */
    DecoderStructInit (&dcInfo);
    if (dcInfo.error) { efree (&inputBuffer); return -1; }

    HuffDecoderInit (&dcInfo);
    if (dcInfo.error) { efree (&inputBuffer); return -1; }

    DecodeImage (&dcInfo, (unsigned short **) &image16, depth);

    /* Free input buffer */
    efree (&inputBuffer);
    
    return 0;
}
