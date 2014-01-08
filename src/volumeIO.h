/*****************************************************************************
 * volumeIO.h is the header file for the volume reading/writing functions
 * for libvp 
 * Oliver Hinds <oph@bu.edu> 2005-04-12
 *
 * 
 *
 *****************************************************************************/

#ifndef VOLUME_IO_H
#define VOLUME_IO_H

#define VOLUME_IO_VERSION_H "$Id: volumeIO.h,v 1.8 2007/08/04 20:29:40 oph Exp $"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>


#include"libvpTypes.h"
#include"ioUtil.h"
#include"volumeUtil.h"
#include"imageIO.h"

//#include"nifti1_io.h"

/**
 * loads a volume based on a passed type 
 */
volume *loadVolume(char *filename, int format);

/**
 * load a volume
 * returns a volume struct
 */
volume *loadVolumeFromImages(char **filenames, int numImages, int format);

/**
 * reads the header of an mgh file
 */
int readMGHHeader(FILE* fp, volume **headerVol);

/**
 * loads a volume from one MGH volume file
 * returns a volume struct
 */
volume *loadMGHVolume(char *filename);

/**
 * writes a volume struct to an MGH volume file
 * returns SUCCESS OR FAILURE
 */
int writeMGHVolume(volume *vol, char *filename);

/**
 * loads a volume from one DICOM file with multiple frames
 * returns a volume struct
 */
volume *loadSingleDICOMVolume(char *filename);

/**
 * writes a volume struct to a DICOM file with multiple frames
 * returns SUCCESS OR FAILURE
 */
int writeSingleDICOMVolume(volume *vol, char *filename);

/**
 * loads a volume from one RAW volume file
 * returns a volume struct
 */
volume *loadRAWVolume(char *filename);

/**
 * writes a volume struct to a RAW volume file
 * returns SUCCESS OR FAILURE
 */
int writeRAWVolume(volume *vol, char *filename);

/**
 * loads a volume from one RAWIV volume file
 * returns a volume struct
 */
volume *loadRAWIVVolume(char *filename);

/**
 * writes a volume struct to a RAW volume file
 * returns SUCCESS OR FAILURE
 */
int writeRAWIVVolume(volume *vol, char *filename);

/**
 * loads a volume from one NIFTI volume file
 * returns a volume struct
 */
volume *loadNiftiVolume(char *filename);

/**
 * writes a volume struct to a NIFTI volume file
 * returns SUCCESS OR FAILURE
 */
int writeNiftiVolume(volume *vol, char *filename);

/**
 * copy image pixels into the appropriate volume slice
 */
void copyPixels(volume* vol, image *img, int sliceIndex);

#endif

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/volumeIO.h,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/

