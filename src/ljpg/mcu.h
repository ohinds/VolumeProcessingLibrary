/*
 * mcu.h --
 *
 * Part of the Independent JPEG Group's software.
 * See the file Copyright for more details.
 *
 */
/*
 * $Id: mcu.h,v 1.1 2005/07/28 19:25:50 oph Exp $
 */
#ifndef _MCU
#define _MCU

/*
 * An MCU (minimum coding unit) is an array of samples.
 */
typedef unsigned short ComponentType; /* the type of image components */
typedef ComponentType *MCU;  /* MCU - array of samples */

extern MCU *mcuTable; /* the global mcu table that buffers the source image */
extern int numMCU;    /* number of MCUs in mcuTable */
extern MCU *mcuROW1,*mcuROW2; /* pt to two rows of MCU in encoding & decoding */

/*
 *--------------------------------------------------------------
 *
 * MakeMCU --
 *
 *      MakeMCU returns an MCU for input parsing.
 *
 * Results:
 *      A new MCU
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */
#define MakeMCU(dcPtr)  (mcuTable[numMCU++])

#endif /* _MCU */
