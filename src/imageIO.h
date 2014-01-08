/*****************************************************************************
 * imageIO.h is the header file for the image reading/writing functions
 * for libvp
 * Oliver Hinds <oph@bu.edu> 2005-04-12
 *
 * 
 *
 *****************************************************************************/

#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#define IMAGE_IO_VERSION_H "$Id: imageIO.h,v 1.7 2007/05/22 19:18:06 oph Exp $"

#include"libvpTypes.h"
#include"libvpUtil.h"
#include"imageUtil.h"
#include"dicom.h" // xmedcom dicom header

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<jpeglib.h>
#include<ctype.h>


/**
 * read a dicom image by name
 * return a pointer to an image struct
 */
image *readDICOM(char* filename);

/**
 * write an image struct to a dicom image file
 * return SUCCESS or FAILURE
 */
int writeDICOM(image *img, char *filename);

/**
 * read a jpeg image by name
 * return a pointer to an image struct
 */
image *readJPEG(char* filename);

/**
 * write an image struct to a jpeg image file
 * only supportes RGB images!!
 * return SUCCESS or FAILURE
 */
int writeJPEG(image *img, char *filename);

/**
 * read a tiff image by name
 * return a pointer to an image struct
 */
image *readTIFF(char* filename);

/**
 * write an image struct to a tiff image file
 * return SUCCESS or FAILURE
 */
int writeTIFF(image *img, char *filename);

/**
 * read a 8 bit per pixel pNm image from an image struct
 * numChannels can be 3 for color, 1 for greyscale
 * binary should be TRUE to write the image in binary format, FALSE for ASCII
 * return a pointer to an image struct
 */
image *readPNM(char *filename);

/**
 * save a 8 bit per pixel pNm image from an image struct
 * return SUCCESS or FAILURE
 */
int writePNM(image *img, char *filename, int binary);

/**
 * read a raw image by name
 * return a pointer to an image struct
 */
image *readDAT(char* filename);

/**
 * write an image struct to a raw image file
 * return SUCCESS or FAILURE
 */
int writeDAT(image *img, char *filename);



#endif

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/imageIO.h,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
