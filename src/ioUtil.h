/*****************************************************************************
 * ioUtil.h is the header file for input/output utility functions for libvp
 * Oliver Hinds <oph@bu.edu> 2005-06-02
 *
 *
 *
 *****************************************************************************/

#ifndef IO_UTIL_H
#define IO_UTIL_H

#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<limits.h>
#include<string.h>

#include"libvpTypes.h"
#include"imageUtil.h"

#ifdef LITTLE_ENDIAN
#define ENDIAN LITTLE
#else
#define ENDIAN BIG
#endif

/** general file io functions **/

/**
 * open a file and return a vpFile descriptor, or NULL on failure
 */
vpFile *vpOpen(const char *filename, const char *mode);

/**
 * closes a vpFile descriptor
 */
void vpClose(vpFile *fp);

/** string manip **/

/**
 * converts an integer to a string
 */
char *itoa(int i);

/** file reading utils **/

/**
 * skip all white space
 */
void skipWhite(FILE* f);

/**
 * skips comments at the top of a file
 */
void skipComments(FILE* f, char symbol);

/**
 * reads all characters until an end line in reached
 */
void readLine(FILE *fp, char *buf);

/**
 * reads n strings from a file stream into an array of strings,
 * returns array for success, NULL for flibvpure
 */
char** readStrings(FILE *fp, int n);

/**
 * gets the number of bytes left in the file stream
 */
long getNumBytesLeftInFile(FILE *fp);

/**
 * reads voxels for a volume from a file stream
 */
int readVolumeVoxels(FILE* fp, volume *vol);

/**
 * reads a certain number of elements into a short buffer, converting from
 * other types, making the best guess at the file's datatype\
 * DONT USE THIS IF THERE IS EXTRA DATA AT THE END OF A FILE
 */
int readDataIntoShortBuffer(FILE *fp, int numElements, unsigned short *buf);

/**
 * reads n big endian byte ordered shorts
 */
int readBigEndianShort16(FILE *fp, int n, unsigned short *dest);

/**
 * writes n big endian byte ordered shorts
 */
int writeBigEndianShort16(FILE *fp, int n, unsigned short *src);

/**
 * reads n big endian byte ordered 3 byte ints
 */
int readBigEndianInt24(FILE *fp, int n, unsigned int *dest);

/**
 * writes n big endian byte ordered 3 byte ints
 */
int writeBigEndianInt24(FILE *fp, int n, unsigned int *src);

/**
 * reads n big endian byte ordered ints
 */
int readBigEndianInt32(FILE *fp, int n, unsigned int *dest);

/**
 * writes n big endian byte ordered ints
 */
int writeBigEndianInt32(FILE *fp, int n, unsigned int *src);

/**
 * reads n big endian byte ordered floats
 */
int readBigEndianFloat32(FILE *fp, int n, float *dest);

/**
 * writes n big endian byte ordered floats
 */
int writeBigEndianFloat32(FILE *fp, int n, float *src);

/**
 * reads n big endian byte ordered doubles
 */
int readBigEndianDouble64(FILE *fp, int n, double *dest);

/**
 * writes n big endian byte ordered doubles
 */
int writesBigEndianDouble64(FILE *fp, int n, double *src);

/**
 * swaps byte order for 16 bit numbers
 */
unsigned short swapByteOrder16(unsigned short n);

/**
 * swaps byte order for 32 bit numbers
 */
unsigned int swapByteOrder32(unsigned int n);

/**
 * swaps byte order for 32 bit floating point numbers
 */
float swapByteOrder32f(float f);

/**
 * swaps byte order for 64 bit floating point numbers
 */
double swapByteOrder64f(double d);

/**
 * convert a byte array into one 32 bit float number
 */
float getFloat32FromBytes(unsigned char byteArray[8]);

/**
 * convert a byte array into one double number
 */
double getDouble64FromBytes(unsigned char byteArray[8]);

/**
 * looks for a valid image or volume type name in a string,
 * returns both the format and type (volume or image)
 */
int getTypeAndFormatFromString(char *str, enum IMAGEFORMAT *format,
                               enum INPUTTYPE *type);

#endif

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/ioUtil.h,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
