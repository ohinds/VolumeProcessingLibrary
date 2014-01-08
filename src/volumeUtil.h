/*****************************************************************************
 * volumeUtil.h is the header file for the utility functions operating on 
 * a volume struct for libvp
 * Oliver Hinds <oph@bu.edu> 2005-04-12
 *
 * 
 *
 *****************************************************************************/

#ifndef VOLUME_UTIL_H
#define VOLUME_UTIL_H

#define VOLUME_UTIL_VERSION_H "$Id: volumeUtil.h,v 1.7 2007/05/22 19:18:06 oph Exp $"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include"libvpTypes.h"
#include"libvpUtil.h"
#include"imageUtil.h"

/**
 * create a volume struct
 */
volume *createVolume(int frames, int rows, int cols, int slices, 
		     int echos, int channels);

/**
 * create a volume struct, but don't allocate the voxels
 * THIS SHOULD ONLY BE USED WHEN HEADER INFO IS BEING READ WITHOUT READING
 * VOXELS! 
 */
volume *createEmptyVolume(int frames, int rows, int cols, int slices, 
		     int echos, int channels);

/**
 * create a volume struct from the header info of another volume
 */
volume *createVolumeFrom(volume *otherVol);

/**
 * free a volume 
 */
void freeVolume(volume *vol);

/**
 * slices a volume along a given dimension
 */
image *sliceVolume(volume *vol, int frame, int dim, int echo);

/**
 * sets a slice of the volume along a given dimension
 */
//int setSlice(volume *vol, int dim, int slice, unsigned short *vox);

/**
 * load an array with the three linear voxel dimensions for row,col,slice
 */
void getVoxelSize(volume *vol, float voxelSizes[3]);

/**
 * get the slice dimension order for a given slice direction
 */
void getSliceDims(int dir, int dims[3]);

/**
 * changes the current frame index, if possible
 * returns SUCCESS or FAILURE
 */
int changeCurFrameBy(volume *vol, int n);

/**
 * changes the current slice index, if possible
 * returns SUCCESS or FAILURE
 */
int changeCurSliceBy(volume *vol, int n);

/**
 * changes the current slice index to the first index, if possible
 * returns SUCCESS or FAILURE
 */
int goToFirstSlice(volume *vol);

/**
 * changes the current slice index to the last index, if possible
 * returns SUCCESS or FAILURE
 */
int goToLastSlice(volume *vol);

/**
 * changes the current slice direction, if possible
 * returns SUCCESS or FAILURE
 */
int swapSliceDir(volume *vol, int dir);

/**
 * increments the brightness adjustment for this volume
 */
int changeVolumeBrightness(volume *vol, float adj);

/**
 * increments the contrast adjustment for this volume
 */
int changeVolumeContrast(volume *vol, float adj);

/**
 * sets the initial volume contrast so the pizel of maximum value is
 * displayed at a given ratio of the maximum pixel intensity
 */
void setInitialVolumeContrast(volume *vol, float ratio);

#endif

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/volumeUtil.h,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
