/*****************************************************************************
 * libvpUtil.c is the source file for utility functions for libvp
 * Oliver Hinds <oph@bu.edu> 2005-06-02
 *
 *
 *
 *****************************************************************************/

#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<limits.h>
#include<string.h>
#include<time.h>

#include"libvpUtil.h"

/* if we want debug info, set this to one in the calling code */
int VP_DEBUG = 0;

/** math **/

int factorial(int n) {
  if(n < 2) return 1;
  return n*factorial(n-1);
}

/**
 * multiply two three d matrices 
 */
void matrixMult3by1(float a[3][3], float b[3], float c[3]) {
  c[0] = a[0][0]*b[0] + a[0][1]*b[1] + a[0][2]*b[2];
  c[1] = a[1][0]*b[0] + a[1][1]*b[1] + a[1][2]*b[2];
  c[2] = a[2][0]*b[0] + a[2][1]*b[1] + a[2][2]*b[2];
}

/**
 * multiply two three d matrices 
 */
void matrixMult3by3(float a[3][3], float b[3][3], float c[3][3]) {
  c[0][0] = a[0][0]*b[0][0] + a[0][1]*b[1][0] + a[0][2]*b[2][0];
  c[1][0] = a[1][0]*b[0][0] + a[1][1]*b[1][0] + a[1][2]*b[2][0];
  c[2][0] = a[2][0]*b[0][0] + a[2][1]*b[1][0] + a[2][2]*b[2][0];

  c[0][1] = a[0][0]*b[0][1] + a[0][1]*b[1][1] + a[0][2]*b[2][1];
  c[1][1] = a[1][0]*b[0][1] + a[1][1]*b[1][1] + a[1][2]*b[2][1];
  c[2][1] = a[2][0]*b[0][1] + a[2][1]*b[1][1] + a[2][2]*b[2][1];

  c[0][2] = a[0][0]*b[0][2] + a[0][1]*b[1][2] + a[0][2]*b[2][2];
  c[1][2] = a[1][0]*b[0][2] + a[1][1]*b[1][2] + a[1][2]*b[2][2];
  c[2][2] = a[2][0]*b[0][2] + a[2][1]*b[1][2] + a[2][2]*b[2][2];
}

/**
 * multiply a 4D matrix and a vector
 */
void matrixMult4by1(float a[4][4], float b[4], float c[4]) {
  c[0] = a[0][0]*b[0] + a[0][1]*b[1] + a[0][2]*b[2] + a[0][3]*b[3];
  c[1] = a[1][0]*b[0] + a[1][1]*b[1] + a[1][2]*b[2] + a[1][3]*b[3];
  c[2] = a[2][0]*b[0] + a[2][1]*b[1] + a[2][2]*b[2] + a[2][3]*b[3];
  c[3] = a[3][0]*b[0] + a[3][1]*b[1] + a[3][2]*b[2] + a[3][3]*b[3];
}

/**
 * multiply a 4D matrix and a vector struct
 */
void matrixMult4byV(float a[4][4], vector b, vector *c) {
  float arr[4], res[4];
  arr[0] = b.x;
  arr[1] = b.y;
  arr[2] = b.z;
  arr[3] = 1.0f;

  matrixMult4by1(a,arr,res);

  c->x = res[0];
  c->y = res[1];
  c->z = res[2];
}

/**
 * multiply two 4D matrices
 */
void matrixMult4by4(float a[4][4], float b[4][4], float c[4][4]) {
  c[0][0] = a[0][0]*b[0][0]+a[0][1]*b[1][0]+a[0][2]*b[2][0]+a[0][3]*b[3][0];
  c[1][0] = a[1][0]*b[0][0]+a[1][1]*b[1][0]+a[1][2]*b[2][0]+a[1][3]*b[3][0];
  c[2][0] = a[2][0]*b[0][0]+a[2][1]*b[1][0]+a[2][2]*b[2][0]+a[2][3]*b[3][0];
  c[3][0] = a[3][0]*b[0][0]+a[3][1]*b[1][0]+a[3][2]*b[2][0]+a[3][3]*b[3][0];

  c[0][1] = a[0][0]*b[0][1]+a[0][1]*b[1][1]+a[0][2]*b[2][1]+a[0][3]*b[3][1];
  c[1][1] = a[1][0]*b[0][1]+a[1][1]*b[1][1]+a[1][2]*b[2][1]+a[1][3]*b[3][1];
  c[2][1] = a[2][0]*b[0][1]+a[2][1]*b[1][1]+a[2][2]*b[2][1]+a[2][3]*b[3][1];
  c[3][1] = a[3][0]*b[0][1]+a[3][1]*b[1][1]+a[3][2]*b[2][1]+a[3][3]*b[3][1];

  c[0][2] = a[0][0]*b[0][2]+a[0][1]*b[1][2]+a[0][2]*b[2][2]+a[0][3]*b[3][2];
  c[1][2] = a[1][0]*b[0][2]+a[1][1]*b[1][2]+a[1][2]*b[2][2]+a[1][3]*b[3][2];
  c[2][2] = a[2][0]*b[0][2]+a[2][1]*b[1][2]+a[2][2]*b[2][2]+a[2][3]*b[3][2];
  c[3][2] = a[3][0]*b[0][2]+a[3][1]*b[1][2]+a[3][2]*b[2][2]+a[3][3]*b[3][2];

  c[0][3] = a[0][0]*b[0][3]+a[0][1]*b[1][3]+a[0][2]*b[2][3]+a[0][3]*b[3][3];
  c[1][3] = a[1][0]*b[0][3]+a[1][1]*b[1][3]+a[1][2]*b[2][3]+a[1][3]*b[3][3];
  c[2][3] = a[2][0]*b[0][3]+a[2][1]*b[1][3]+a[2][2]*b[2][3]+a[2][3]*b[3][3];
  c[3][3] = a[3][0]*b[0][3]+a[3][1]*b[1][3]+a[3][2]*b[2][3]+a[3][3]*b[3][3];
}

/**
 * transpose a 4D matrix
 */
void transposeMatrix4by4(float a[4][4], float trans[4][4]) {
  int i, j;
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      trans[j][i] = a[i][j];
    }
  }
}

/**
 * invert a 4D matrix
 */
void invertMatrix4by4(float a[4][4], float b[4][4]) {
  // make the gsl_matrix
  gsl_matrix *A = gsl_matrix_alloc(4,4);
  gsl_permutation *p = gsl_permutation_alloc(4);
  int s;

  gsl_matrix *M = gsl_matrix_alloc(4,4);

  /* initialize A */
  int i,j;
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      gsl_matrix_set(A,i,j,a[i][j]);
    }
  }

  gsl_linalg_LU_decomp(A,p,&s);
  gsl_linalg_LU_invert(A,p,M);

  /* copy to b */
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      b[i][j] = gsl_matrix_get(M,i,j);
    }
  }

  /* free gsl matrixes */
  gsl_matrix_free(A);
  gsl_matrix_free(M);
  gsl_permutation_free(p);
}

/** general **/

/**
 * get a string representing the current time in the same format as 
 * dicom filenames
 */
char *getTimeString() {
  time_t timelong = time(NULL);
  struct tm *timestruct = localtime(&timelong);

  char *timestring = (char*) malloc(VP_MAX_STR_LEN*sizeof(char));

  sprintf(timestring, "%04d_%02d_%02d_%02d_%02d_%02d",
	  timestruct->tm_year+1900,
	  timestruct->tm_mon+1,
	  timestruct->tm_mday,
	  timestruct->tm_hour,
	  timestruct->tm_min,
	  timestruct->tm_sec);

  return timestring;
}

/**
 * prints a message about unsupported functionality and returns FAILURE
 */
int notSupported() {
  fprintf(stderr,"this functionality is not yet supported.\n");
  return VP_FAILURE;
}

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/libvpUtil.c,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/

