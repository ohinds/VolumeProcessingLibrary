/*
 * mcu.c --
 *
 * Support for MCU allocation, deallocation, and printing.
 *
 */
/*
 * $Id: mcu.c,v 1.1 2005/07/28 19:25:50 oph Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpeg.h"
#include "mcu.h"
#include "proto.h"

MCU *mcuTable=NULL;     /* the global mcu table that buffers the source image */

MCU *mcuROW1=NULL;      /* point to two rows of MCU in encoding & decoding */
MCU *mcuROW2=NULL;

int numMCU=0;           /* number of MCUs in mcuTable */

/*
 *--------------------------------------------------------------
 *
 * MakeMCU, InitMcuTable --
 *
 *        InitMcuTable does a big malloc to get the amount of memory
 *        we'll need for storing MCU's, once we know the size of our
 *        input and output images.
 *        MakeMCU returns an MCU for input parsing.
 *
 * Results:
 *        A new MCU
 *
 * Side effects:
 *        None.
 *
 *--------------------------------------------------------------
 */
void InitMcuTable (int numMCU,int compsInScan)
{
    int i, mcuSize;
    char *buffer;

    /*
     * Compute size of on MCU (in bytes).  Round up so it's on a
     * boundary for any alignment.  In this code, we assume this
     * is a whole multiple of sizeof(double).
     */
    mcuSize = compsInScan * sizeof(ComponentType);
    mcuSize = JroundUp(mcuSize,sizeof(double));

    /*
     * Allocate the MCU table, and a buffer which will contain all
     * the data.  Then carve up the buffer by hand.  Note that
     * mcuTable[0] points to the buffer, in case we want to free
     * it up later.
     */
    mcuTable = (MCU *)malloc(numMCU * sizeof(MCU));
    if (mcuTable==NULL)
       fprintf(stderr,"Not enough memory for mcuTable\n");
    buffer = (char *)malloc((unsigned)(numMCU * mcuSize));
    if (buffer==NULL)
       fprintf(stderr,"Not enough memory for buffer\n");
    for (i=0; i<numMCU; i++) {
        mcuTable[i] = (MCU)(buffer + i*mcuSize);
    }
}

#define MakeMCU(dcPtr)                (mcuTable[numMCU++])


/*
 *--------------------------------------------------------------
 *
 * FreeMcuTable --
 *
 * Frees up the allocated table
 *
 * Results:
 * None.
 *
 * Side effects:
 * None.
 *
 *--------------------------------------------------------------
 */
void FreeMcuTable(void)
{
         free( mcuTable );
}

/*
 *--------------------------------------------------------------
 *
 * PrintMCU --
 *
 *        Send an MCU in quasi-readable form to stdout.
 *
 * Results:
 *        None.
 *
 * Side effects:
 *        None.
 *
 *--------------------------------------------------------------
 */
void PrintMCU (int compsInScan, MCU mcu)
{
    ComponentType r;
    int b;
    static int callCount;

    for (b=0; b<compsInScan; b++) {
        callCount++;
        r = mcu[b];
        printf ("%d: %d ", callCount, r);
        printf ("\n");
    }
}
