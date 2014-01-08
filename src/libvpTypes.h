/*****************************************************************************
 * libvpTypes.h is the header file containing type declarations for libvp
 * Oliver Hinds <oph@bu.edu> 2004-06-07
 *
 *
 *
 *****************************************************************************/

#ifndef VP_TYPES_H
#define VP_TYPES_H

#define VP_TYPES_VERSION_H "$Id: libvpTypes.h,v 1.17 2007/08/04 20:29:40 oph Exp $"

#define VP_MAX_STR_LEN 255

#define VP_FAILURE -1
#define VP_SUCCESS 1

#define VP_TRUE 1
#define VP_FALSE 0

#define VP_PI 3.14159265358979
#define VP_TOL 0.00000001

#define VP_MIN 0
#define VP_MAX 1

#include<stdlib.h>
#include<stdio.h>
#include<zlib.h>

/* endianness */
enum ENDIANNES {LITTLE, BIG};

/** byte reordering unions **/
typedef union {
  double d;
  float f[2];
  unsigned short s[4];
  unsigned char bytes[8];
} doubleUnion;

typedef union {
  float f;
  unsigned short s[2];
  unsigned char bytes[4];
} floatUnion;

/* file io */
typedef struct {
  FILE *fp;
  gzFile zfp;
  int open;
} vpFile;

/** list **/

/* list types */
enum LISTTYPE {LIST, STACK, QUEUE, MINHEAP, MAXHEAP};

/* node for list */
typedef struct {
  /* pointer to the data */
  void *data;

  int delete;

  /* value of the node */
  double value;

  /* pointer to next node in list */
  struct listNode *next;
} listNode;

/* list type */
typedef struct {
  listNode *head;
  listNode *tail;
  int len;

  enum LISTTYPE type;
} list;

/** util **/

/* vector datatype */
typedef struct {
  double x,y,z;
} vector;

/* quadrilateral datatype */
typedef struct {
  vector v1, v2, v3, v4;
} quadri;

/* input types */
enum INPUTTYPE {INVALID_INPUT_TYPE, VP_VOLUME, VP_IMAGE, VP_NOIMAGES};

/* image */
enum IMAGEFORMAT {INVALID_FORMAT, DICOM, JPEG, TIFF, PNM, MGH, RAW, RAWIV, NIFTI, DAT};
enum IMAGETYPE {UCHAR, INT, LONG, FLOAT, SHORT, BITMAP};

typedef struct {
  unsigned int width, height;
  enum IMAGETYPE type;
  unsigned short *pixels;

  /* coordinate system */
  float pix2wrld[4][4];

  /* misc info */
  char filename[VP_MAX_STR_LEN];
  int padX,padY;
  int numChannels;

  /* display vars */
  unsigned int texture;
  float padRatio[2];

  /* image processing vars */
  float brightnessAdjust;
  float contrastAdjust;

} image;

/* mri parameters */
typedef struct {
  float tr;        /* in sec */
  float flipangle; /* in degrees */
  float te;        /* in sec */
  float ti;        /* in sec */
  float fov;       /* */
} mriParameters;

/* volume */
#define _D 6
enum SLICEDIRECTION {FRAME, ROW, COL, SLICE, ECHO, CHANNEL};

/* actual volume struct */
typedef struct {
  /* properties of the volume */
  unsigned int size[_D]; /* frames, rows, cols, slices, echos, channels */
  unsigned int minmaxSize[2];
  unsigned short *voxels;
  enum IMAGETYPE type;
  int actualBPP;

  /* filename */
  char filename[VP_MAX_STR_LEN];

  /* world coordinate system transformation matrix */
  float vox2wrld[4][4]; /* see help load_mgh */
  //float Mdc[3][3]; /* keeps sign and dimensional info */

  /* mri parameters */
  struct mriParameters *mriParms;

  /* display vars */
  enum SLICEDIRECTION sliceDir;
  unsigned int tex3d;
  unsigned int textures[_D];
  float padRatios[_D][2];
  int selectedVoxel[_D];

  /* image processing vars */
  float brightnessAdjust;
  float contrastAdjust;

} volume;

/** colors **/

/* type */
typedef const float* color;

extern const int NUMVPCOLORS;
extern const int NUMTACKVPCOLORS;

extern const int RED;
extern const int BLUE;
extern const int GREEN;
extern const int BLACK;
extern const int DARK_GRAY;
extern const int PURPLE;
extern const int BLACK;
extern const int BROWN;
extern const int WHITE;
extern const int GRAY;
extern const int LIGHT_GRAY;
extern const int LIGHT_BLUE;
extern const int ORANGE;

extern const int GRAYED_LIGHT_ORANGE;
extern const int GRAYED_DARK_ORANGE;

/* array of the color vectors, to be indexed by the color names */
extern const float *VPCOLORS[];

#endif

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/libvpTypes.h,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
