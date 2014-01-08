/*****************************************************************************
 * imageUtil.h is the header file for the misc image utility functions
 * for libvp
 * Oliver Hinds <oph@bu.edu> 2005-04-12
 *
 *
 *
 *****************************************************************************/

#ifndef IMAGE_UTIL_H
#define IMAGE_UTIL_H

#define IMAGE_UTIL_VERSION_H "$Id: imageUtil.h,v 1.5 2007/07/20 05:08:47 oph Exp $"

#include<limits.h>

#include"libvpTypes.h"
#include"libvpUtil.h"

/**
 * create an image struct
 */
image *createImage(int width, int height, int numChannels);

/**
 * free an image struct
 */
void freeImage(image *img);

/** image resizing functions **/

/**
 * resizes an image using bilinear interpolation
 */
int resizeImageBilinear(image *img, int newW, int newH);

/**
 * resizes an image using bicubic spline interpolation
 */
int resizeImageBicubic(image *img, int newW, int newH);

/** image bit depth conversion functions **/

/**
 * bitDepthConvert a 32 bpp image to a 16 bpp image
 */
void bitDepthConvert32to16(unsigned int *buf32, unsigned short *buf16,
                           int numPix);

/**
 * bitDepthConvert a 16 bpp image to a 12 bpp image
 */
void bitDepthConvert16to32(unsigned short *buf16, unsigned int *buf32,
                           int numPix);

/**
 * bitDepthConvert a 32 bpp floating point image to a 16 bpp image
 */
void bitDepthConvert32fto16(float *buf32, unsigned short *buf16, int numPix);

/**
 * bitDepthConvert a 16 bpp image 32 bpp floating point image
 */
void bitDepthConvert16to32f(unsigned short *buf16, float *buf32, int numPix);

/**
 * bitDepthConvert a 32 bpp image to a 8 bpp image
 */
void bitDepthConvert32to8(unsigned int *buf32, unsigned char *buf8, int numPix);

/**
 * bitDepthConvert a 8 bpp image to a 32 bpp image
 */
void bitDepthConvert8to32(unsigned char *buf8, unsigned int *buf32, int numPix);

/**
 * bitDepthConvert a 32 bpp floating point image to a 8 bpp image
 */
void bitDepthConvert32fto8(float *buf32, unsigned char *buf8, int numPix);

/**
 * bitDepthConvert a 8 bpp image to a 32 bpp floating point image
 */
void bitDepthConvert8to32f(unsigned char *buf8, float *buf32, int numPix);

/**
 * bitDepthConvert a 16 bpp image to a 8 bpp image
 */
void bitDepthConvert16to8(unsigned short *buf16, unsigned char *buf8,
                          int numPix);

/**
 * bitDepthConvert a 8 bpp image to a 16 bpp image
 */
void bitDepthConvert8to16(unsigned char *buf8, unsigned short *buf16,
                          int numPix);


/** whole image transformations **/

/**
 * increments the brightness adjustment for this image
 */
int changeImageBrightness(image *img, float adj);

/**
 * increments the contrast adjustment for this image
 */
int changeImageContrast(image *img, float adj);

/**
 * checks if two images are "compatible"
 */
int imagesCompatible(image *img1, image *img2);

/**
 * subtracts two images
 */
image *subtractImages(image *img1, image *img2);

/**
 * flip an image
 * reverse ordering of rows in buffer, since jpeg storage is flipped
 * with respect to openGL's buffer format.
 */
void flipImage(image *img);

/**
 * pads an image so dimensions power of 2, doesn't change w and h, but
 * gives the number of pixels added in each dimension
 */
void padImagePow2(image* img);

/**
 * trim the image down to the size in the image struct from the size specified
 */
void trimImage(image* img, int pixAddX);

/**
 * flattens an image by ignoring the alpha channel
 */
void flattenImage(image* img);


#endif

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/imageUtil.h,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
