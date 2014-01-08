/****************************r
 * libdicom - MDC extensions *
 *****************************/
/*
 * $Id: decomp.c,v 1.4 2006/09/28 15:27:56 oph Exp $
 */

/*
 * Contributions by Jaslet Bertrand
 *
 * for handling encapsulated pixeldata:
 *
 *  - RLE
 *
 *  - LossLess JPEG
 */

#include "dicom.h"

#ifdef MDC_SUPPORT_LJPG
 #include "jpeg.h"
 #include "jpegless.h"
#endif

#define MDC_MAX_RLE_SEGMENTS  4L   /* max of 32 bits (ARGB) images supported */

/*******
 * RLE *
 *******/

static void mdc_dicom_decodeRLE_segment(U16 *, U8 *, U32, U32 , U32);

/*
 * gets and decode an RLE pixel data element
 * return : the image 
 */
S16 mdc_dicom_decomp_rle(FILE *fp, U16 *image16, U32 length)
{
  U32 	numberSegments, i;
  U8	*rle;
  long	offset[MDC_MAX_RLE_SEGMENTS + 1], rlelen, skip;

  dicom_log(DICOM_DEBUG,"mdc_dicom_decomp_rle()");
  
  /* for each image we have:                                       */
  /* 0xFFFE 0xE000 length RLE_header RLE_segment1 RLE_segment2 ... */
  /* length is 4 bits / image                                      */
   
  /* read 4 chars from the file = number of segments */
  fread(&numberSegments,4,1,fp);
  if (dicom_check(-1))
  {
    dicom_log(ERROR,"RLE - Failure numberSegments");
    return -1;
  }
  dicom_swap(&numberSegments,4);
  if (numberSegments > MDC_MAX_RLE_SEGMENTS) 
  {
    dicom_log(ERROR,"RLE - Maximum of 32 bits images supported"); 
    return -1; /* allow 8, 16, 24 & 32 bits images, 8 bits per segment */
  }
 
  /* read offset0, offset1, offset2, ... */
  for (i=0; i < numberSegments; i++) {
     fread(&offset[i],4,1,fp);
     if (dicom_check(-1))
     {
       dicom_log(ERROR,"RLE - Failure offsets");
       return -1;
     }
     dicom_swap(&offset[i],4);
  }
  /* skip rest of header */
  skip = 60 - (numberSegments * 4);
  fseek(fp, skip, SEEK_CUR);
  if (dicom_check(-1))
  {
    dicom_log(ERROR,"RLE - Failure header skip");
    return -1;
  }

  offset[numberSegments] = length; /* needed for offset last segment */

  /* read all segments */
  for (i=0; i < numberSegments; i++)
  {
    /* read rle image */
    rlelen = offset[i+1] - offset[i];
    rle	   = (U8*)malloc((U32)(rlelen + 10L));
    if (rle)
    {
      /* extract the image from the file  */
      fread((void *)rle, (unsigned)rlelen, 1L, fp);
      if (ferror(fp)) {
        dicom_log(ERROR,"RLE - Failure image read");
        return -2;
      }
      mdc_dicom_decodeRLE_segment(image16,rle
                                         ,(unsigned)rlelen,numberSegments,i);
      /* delete buffer */
      free(rle);
    }
    else
    {
      dicom_log(ERROR,"RLE - Out of memory");
      return -3;
    }
  }
 
  return 0;
    
}

/* 
 *  decode a RLE segment							
 *  image  : pointer on real image (8 or 16 bits)			
 *  rle    : pointer on rle buffer (8bits)				
 *  length : length of rle buffer					
 *  segtot : total number of segments				
 *  segnb  : number of current segment (zero based !)
 */
void mdc_dicom_decodeRLE_segment(U16 *image, U8 *rle, U32 length,U32 segtot, U32 segnb)
{
  U32  j, indj;
  U8   *pix, val;
  U16  code; /* prevent warning: actually signed char >=128) = (256-code) */
  U16  ii, iimax;

  dicom_log(DICOM_DEBUG,"mdc_dicom_decodeRLE_segment()");

  /* convert rle into real image      */
  pix  = (U8*) image;
  /* initial start number, zero based */
  /* segment 1st=0, 2nd=1, 3rd=2, ... */
  indj = segnb;
  for (j = 0L; j < length; )
  {
    code = (U16) rle [j];
    j++; /* yes, I know but do not move it */
    /* sequence of different bytes */
    if (code == 0)
    {
      if (j < length - 1)
        pix [indj] = rle [j++];
      indj += segtot;
    }
    else if ((code > 0) && (code <= 127))
    {
      for (ii = 0; ii < (code + 1); ii++)
      {
         if (j == length) break;
         pix [indj] = rle [j++];
         indj += segtot;
      }
    }
    /* repetition of the same byte */
    else if ((code <= 255) && (code > 128))
    {
      val = rle [j++];
      iimax = 256-code;
      for (ii = 0; ii <= iimax; ii++)
      {
        pix [indj] = val;
        indj += segtot;
      }
    }
  } /* for */
}

/*****************
 * LossLess JPEG *
 *****************/

S16 mdc_dicom_decomp_ljpg(FILE *fp, U16 *image16, U32 length, U32 depth)
{
#if MDC_SUPPORT_LJPG
  return(JPEGLosslessDecodeImage(fp,image16,(signed)depth,(signed)length));
#else
  return(-1);
#endif
}
