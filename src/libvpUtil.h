/*****************************************************************************
 * libvpUtil.h is the header file for misc utility functions for libvp
 * Oliver Hinds <oph@bu.edu> 2005-06-02
 *
 * 
 *
 *****************************************************************************/

#ifndef VP_UTIL_H
#define VP_UTIL_H

#include<math.h>
#include<jpeglib.h>
#include<ctype.h>

#include"libvpTypes.h"
#include"list.h"

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

/******************************************************
 * constants and types
 ******************************************************/

/** math functions **/
#define max(a,b) (a >= b ? a : b)
#define min(a,b) (a <= b ? a : b)

#define length(a,b) sqrt(pow(a.x-b.x,2) + pow(a.y-b.y,2))

/* if we want debug info, set this to one in the calling code */
extern int VP_DEBUG;

/** math **/
int factorial(int n);

/**
 * multiply two three d matrices 
 */
void matrixMult3by1(float a[3][3], float b[3], float c[3]);

/**
 * multiply two three d matrices 
 */
void matrixMult3by3(float a[3][3], float b[3][3], float result[3][3]);

/**
 * multiply a 4D matrix and a vector
 */
void matrixMult4by1(float a[4][4], float b[4], float c[4]);

/**
 * multiply a 4D matrix and a vector struct
 */
void matrixMult4byV(float a[4][4], vector b, vector *c);

/**
 * multiply two 4D matrices
 */
void matrixMult4by4(float a[4][4], float b[4][4], float result[4][4]);

/**
 * transpose a 4D matrix
 */
void transposeMatrix4by4(float a[4][4], float trans[4][4]);

/**
 * invert a matrix
 */
void invertMatrix4by4(float a[4][4], float b[4][4]);

/** general **/

/**
 * get a string representing the current time in the same format as 
 * dicom filenames
 */
char *getTimeString();

/**
 * prints a message about unsupported functionality and returns FAILURE
 */
int notSupported();

#endif

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/libvpUtil.h,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
