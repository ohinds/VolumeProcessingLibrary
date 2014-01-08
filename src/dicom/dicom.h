/*************************
 * libdicom by Tony Voet *
 *************************/
/*
 * $Id: dicom.h,v 1.2 2005/07/29 12:42:18 oph Exp $
 */

#ifndef __LIBDICOM__

#include <stdio.h>
#include <stdlib.h>

/* change code for MedCon */
#define MEDCON_INTEGRATED 0

/* eNlf: BEGIN - add macro for safe freeing of pointers */
#define eNlfSafeFree(p)          { if (p != NULL) free(p); p=NULL; }
/* eNlf: END   - add macro for safe freeing of pointers */

/* disable/enable DICOM debugging */
#define MDC_DICOM_DICOM_DEBUG   0

#define S8	signed char
#define S16	signed short
#define S32	signed int

#define U8	unsigned char
#define U16	unsigned short
#define U32	unsigned int

/*******
 * fix *
 *******/

typedef enum
{
  MDC_FIX_EZDICOM=1
}
WORKAROUND;

extern WORKAROUND dicom_workaround;

/*******
 * log *
 *******/

typedef enum
{
  EMERGENCY,
  ALERT,
  CRITICAL,
  ERROR,
  WARNING,
  NOTICE,
  INFO,
  DICOM_DEBUG
}
CONDITION;

extern CONDITION dicom_log_level;

void	dicom_log_name(char *);
int	dicom_log_open(const char *);
void	dicom_log(CONDITION,const char *);
int	dicom_log_close(void);

/**********
 * single *
 **********/

typedef struct
{
  U16 size,bit;

  union
  {
    U16	u16;
    S16	s16;
  }
  threshold;

  union
  {
    U16	*u16;
    S16	*s16;
  }
  data;
}
CLUT;

typedef struct
{
  enum
  {
    MONOCHROME2,
    MONOCHROME1,
    PALETTE_COLOR,
    RGB,
    HSV,
    ARGB,
    CMYK,
    UNKNOWN
  }
  photometric;

  int	frames;
  U16	w,h,samples,alloc,bit,high,sign,planar;
  CLUT	clut[3];
  void	*data;
}
SINGLE;

SINGLE	*dicom_single(void);
void	dicom_single_free(void);

/*********
 * basic *
 *********/

typedef struct
{
  U16 group,element;
}
TAG;

typedef enum
{
  AE=('A'<<8)|'E',
  AS=('A'<<8)|'S',
  AT=('A'<<8)|'T',
  CS=('C'<<8)|'S',
  DA=('D'<<8)|'A',
  DS=('D'<<8)|'S',
  DT=('D'<<8)|'T',
  FL=('F'<<8)|'L',
  FD=('F'<<8)|'D',
  IS=('I'<<8)|'S',
  LO=('L'<<8)|'O',
  LT=('L'<<8)|'T',
  OB=('O'<<8)|'B',
  OW=('O'<<8)|'W',
  PN=('P'<<8)|'N',
  SH=('S'<<8)|'H',
  SL=('S'<<8)|'L',
  SQ=('S'<<8)|'Q',
  SS=('S'<<8)|'S',
  ST=('S'<<8)|'T',
  TM=('T'<<8)|'M',
  UI=('U'<<8)|'I',
  UL=('U'<<8)|'L',
  US=('U'<<8)|'S',
  UN=('U'<<8)|'N',
  UT=('U'<<8)|'T',
/* special tag (choices) */
  ox=('o'<<8)|'x'
}
VR;

typedef struct
{
  U16	group,element;
  VR	vr;
  U32	length;

  union
  {
    TAG		*AT;
    double	*FD;
    float	*FL;
    U32		*UL;
    S32		*SL;
    U16		*OW,*US;
    S16		*SS;
    U8		*OB;
    char	**AE,**AS,**CS,**DA,**DS,**DT,**IS,
		**LO,*LT,**PN,**SH,*ST,**TM,**UI, *UT;
    void	*SQ,*UN;
  }
  value;

  U32	vm;
  int	encapsulated;
  U8	sequence;
  TAG   sqtag; 
}
ELEMENT;

extern char dicom_version[],**dicom_transfer_syntax;

#if MEDCON_INTEGRATED
void    dicom_init(FILE *fp);
#endif
int	dicom_open(const char *);
ELEMENT	*dicom_element(void);
int	dicom_skip(void);
int	dicom_load(VR);
void	dicom_clean(void);
int	dicom_close(void);
void    dicom_swap(void *,int);
int     dicom_check(int);

/* eNlf: BEGIN -- changes for integration in MedCon */
int  mdc_dicom_decompress(SINGLE *, ELEMENT *);
int  mdc_dicom_skip_sequence(ELEMENT *);
int  mdc_dicom_fseek(U32, int);
U32  mdc_dicom_ftell(void);
int  mdc_dicom_load(VR);
void mdc_dicom_switch_endian(void);
void mdc_dicom_switch_syntax_endian(void);
/* eNlf: END   -- changes for integration in MedCon */

/**************
 * dictionary *
 **************/

typedef enum
{
  EVEN,
  ODD,
  ANY
}
MATCH;

typedef struct
{
  U16	group,group_last;
  MATCH	group_match;
  U16	element,element_last;
  MATCH	element_match;
  VR	vr;
  char	*description;
}
DICTIONARY;

DICTIONARY *dicom_query(ELEMENT *);
DICTIONARY *dicom_private(DICTIONARY *,ELEMENT *);

/*******
 * bit *
 *******/

void	dicom_bit(void *);

void	dicom_8_skip(int);
void	dicom_16_skip(int);
void	dicom_32_skip(int);

U32	dicom_8_read(int);
U32	dicom_16_read(int);
U32	dicom_32_read(int);
U16 mdc_dicom_12_unpack(int);

/*********
 * image *
 *********/

typedef struct
{
  int	rgb;
  U16	frames,w,h;

  union
  {
    U16	*gray;
    U8	*rgb;
  }
  data;
}
IMAGE;

IMAGE	*dicom_new(int,U16,U16,U16);
int	dicom_read(const char *,IMAGE **,int *,int,int);
void	dicom_free(IMAGE *,int);

int	dicom_write(const char *,const IMAGE *);
/* eNlf: BEGIN - comment out unwanted stuff */
/* int	dicom_write_ascii(const char *,const IMAGE *,int); */
/* int	dicom_write_jpeg(const char *,const IMAGE *,int);  */
/* int	dicom_write_eps(const char *,const IMAGE *);       */
/* eNlf: END   - comment out unwanted stuff */          
/*************
 * transform *
 *************/

int	dicom_alloc(SINGLE *);
int	dicom_sign(SINGLE *);
int	dicom_planar(SINGLE *);
int	dicom_shift(SINGLE *);

IMAGE	*dicom_transform(SINGLE *,int);

/********
 * zoom *
 ********/

IMAGE *dicom_zoom(const IMAGE *,int,int,int);

/***********
 * process *
 ***********/

void	dicom_max(IMAGE *);
void	dicom_invert(IMAGE *);
void	dicom_voi(IMAGE *,U16,U16);
void	dicom_gray(IMAGE *);
void    dicom_color(IMAGE *image, U8 *palette, U8 dither, char *(*reduce)());

void	dicom_hsv(U16,U16,U16,U8 *);
IMAGE	*dicom_merge(const IMAGE *,const IMAGE *,U16);

/**************
 * MDC decomp *
 **************/

S16 mdc_dicom_decomp_rle(FILE *, U16 *, U32);
S16 mdc_dicom_decomp_ljpg(FILE *, U16 *, U32, U32);

#define __LIBDICOM__
#endif
