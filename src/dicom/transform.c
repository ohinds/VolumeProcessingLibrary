/*************************
 * libdicom by Tony Voet *
 *************************/
/*
 * $Id: transform.c,v 1.2 2005/07/29 12:42:18 oph Exp $
 */

#include <stdlib.h>
#include <string.h>
#include "dicom.h"

static U16 dicom_clut(const CLUT *,U16);

/*********
 * alloc *
 *********/

int dicom_alloc(SINGLE *single)
{
  U32	length,l;
  U16	magic=0x1234,*data,*d;
  int	high,bit,low;

  dicom_log(DICOM_DEBUG,"dicom_alloc()");

  if (!single)
  {
    dicom_log(ERROR,"No image given");
    return -1;
  }

  if (single->alloc>16)
    dicom_log(WARNING,"Large BitsAllocated");

  length=single->frames*single->w*single->h*single->samples;

  data=malloc(length*2);
  if (!data)
  {
    dicom_log(ERROR,"Out of memory");
    return -2;
  }

  high=single->alloc-single->high-1;
  bit=single->bit;
  low=single->high+1-bit;

  d=data;

  dicom_bit(single->data);

  if ( *((U8*)&magic)==0x12 )
    if (single->alloc==12)
      for (l=length; l; l-=2)
      {
        *d++=mdc_dicom_12_unpack(1);
        *d++=mdc_dicom_12_unpack(2);
      }
    else 
      for (l=length; l; l--)
      {
        dicom_32_skip(high);
        *d++=dicom_32_read(bit);
        dicom_32_skip(low);
      }
  else
    if (single->alloc==16)
      for (l=length; l; l--)
      {
	dicom_16_skip(high);
	*d++=dicom_16_read(bit);
	dicom_16_skip(low);
      }
    else if (single->alloc==12)
      for (l=length; l; l-=2)
      {
        *d++=mdc_dicom_12_unpack(1);
        *d++=mdc_dicom_12_unpack(2);
      }
    else
      for (l=length; l; l--)
      {
	dicom_8_skip(high);
	*d++=dicom_8_read(bit);
	dicom_8_skip(low);
      }

  eNlfSafeFree(single->data);
  single->data=data;

  single->alloc=16;
  single->high=single->bit-1;

  return 0;
}

/********
 * sign *
 ********/

int dicom_sign(SINGLE *single)
{
  int	edge,i;
  U32	length,l;
  U16	*d;

  dicom_log(DICOM_DEBUG,"dicom_sign()");

  if (!single)
  {
    dicom_log(ERROR,"No image given");
    return -1;
  }

  if (!single->sign)
    return 0;

  if (single->alloc!=16)
  {
    dicom_log(ERROR,"BitsAllocated != 16");
    return -2;
  }

  if (single->high!=single->bit-1)
    dicom_log(WARNING,"Wrong HighBit");

  edge=1<<(single->bit-1);

  length=single->frames*single->w*single->h*single->samples;
  d=single->data;

  for (l=length; l; l--,d++)
    if (*d<edge)
      *d+=edge;
    else
      *d-=edge;

  switch(single->photometric)
  {
  case PALETTE_COLOR :
  case ARGB :
    for (i=0; i<3; i++)
      if (single->clut[i].threshold.u16<edge)
        single->clut[i].threshold.u16+=edge;
      else
        single->clut[i].threshold.u16-=edge;

    for (i=0; i<3; i++)
      if (!single->clut[i].data.u16)
	dicom_log(ERROR,"Missing CLUT");
      else
      {
        edge=1<<(single->clut[i].bit-1);
        d=single->clut[i].data.u16;

        for (l=single->clut[i].size; l; l--,d++)
	  if (*d<edge)
	    *d+=edge;
	  else
	    *d-=edge;
      }
    break;

  default :
    break;
  }

  single->sign=0;

  return 0;
}

/**********
 * planar *
 **********/

int dicom_planar(SINGLE *single)
{
  int	i,j;
  U32	length,l;
  U16	*frame_s,*frame_d,*s,*d;

  dicom_log(DICOM_DEBUG,"dicom_planar()");

  if (!single)
  {
    dicom_log(ERROR,"No image given");
    return -1;
  }

  if (single->samples<=1)
    return 0;

  if (!single->planar)
    return 0;

  if (single->alloc!=16)
  {
    dicom_log(ERROR,"BitsAllocated != 16");
    return -2;
  }

  length=single->w*single->h;

  frame_d=malloc(length*single->samples*2);
  if (!frame_d)
  {
    dicom_log(ERROR,"Out of memory");
    return -3;
  }

  for (i=0; i<single->frames; i++)
  {
    frame_s=(U16*)single->data+i*length*single->samples;
    s=frame_s;

    for (j=0; j<single->samples; j++)
    {
      d=frame_d+j;

      for (l=length; l; l--)
      {
        *d=*s++;
        d+=single->samples;
      }
    }

    memcpy(frame_s,frame_d,length*single->samples*2);
  }

  eNlfSafeFree(frame_d);

  single->planar=0;

  return 0;
}

/*********
 * shift *
 *********/

int dicom_shift(SINGLE *single)
{
  int	shift,i;
  U32	length,l;
  U16	*d;

  dicom_log(DICOM_DEBUG,"dicom_shift()");

  if (!single)
  {
    dicom_log(ERROR,"No image given");
    return -1;
  }

  if (single->photometric==MONOCHROME1 || single->photometric==MONOCHROME2)
    return 0;

  if (single->alloc!=16)
  {
    dicom_log(ERROR,"BitsAllocated != 16");
    return -2;
  }

  switch(single->photometric)
  {
  default :
    shift=15-single->high;

    if (!shift)
      return 0;

    length=single->frames*single->w*single->h*single->samples;
    d=single->data;

    for (l=length; l; l--)
      *d++<<=shift;

    single->high=15;
    break;

  case ARGB :
    shift=15-single->high;

    if (shift)
    {
      length=single->frames*single->w*single->h;
      d=single->data;

      for (l=length; l; l--)
      {
        d++;
	*d++<<=shift;
	*d++<<=shift;
	*d++<<=shift;
      }

      single->high=15;
    }

  case PALETTE_COLOR :
    for (i=0; i<3; i++)
    {
      shift=16-single->clut[i].bit;

      if (!shift)
	continue;

      d=single->clut[i].data.u16;

      for (l=single->clut[i].size; l; l--)
	*d++<<=shift;

      single->clut[i].bit=16;
    }
  }

  return 0;
}

/*************
 * transform *
 *************/

IMAGE *dicom_transform(SINGLE *single,int parametric)
{
  static IMAGE image;

  U32	length,l;
  U16	*s;
  U8	*d;

  dicom_log(DICOM_DEBUG,"dicom_transform()");

  if (!single)
  {
    dicom_log(ERROR,"No image given");
    return 0L;
  }

  if (dicom_alloc(single))
    return 0L;

  switch(single->photometric)
  {
    case MONOCHROME1:
    case MONOCHROME2:
        /* keep original values, either negative and/or quantified */
        break;
    default:
        /* make positive, colored files */
        if (dicom_sign(single))
          return 0L;
  }

  if (dicom_planar(single))
    return 0L;

  if (dicom_shift(single))
    return 0L;

  memset(&image,0,sizeof(IMAGE));

  image.frames=single->frames;
  image.w=single->w;
  image.h=single->h;

  switch(single->photometric)
  {
  case MONOCHROME1 :
  case MONOCHROME2 :
    image.rgb=0;
    image.data.gray=single->data;
    single->data=0L;

    if (parametric)
      return &image;

    dicom_max(&image);
     
    if (single->photometric==MONOCHROME1)
      dicom_invert(&image);

    return &image;

  case PALETTE_COLOR :
  case ARGB :
    if (!single->clut[0].data.u16 ||
        !single->clut[1].data.u16 ||
        !single->clut[2].data.u16)
    {
      dicom_log(ERROR,"Missing CLUT");
      return 0L;
    }
    break;

  default :
    break;
  }

  image.rgb=-1;
  image.data.rgb=malloc((unsigned)(image.frames*image.w*image.h)*3U);
  if (!image.data.rgb)
  {
    dicom_log(ERROR,"Out of memory");
    return 0L;
  }

  length=image.frames*image.w*image.h;

  s=(U16*)single->data;
  d=image.data.rgb;

  switch(single->photometric)
  {
  case PALETTE_COLOR :
    for (l=length; l; l--)
    {
      *d++=dicom_clut(single->clut,  *s)>>8;
      *d++=dicom_clut(single->clut+1,*s)>>8;
      *d++=dicom_clut(single->clut+2,*s)>>8;
      s++;
    }
    break;

  case RGB :
    for (l=length*3; l; l--)
      *d++=*s++>>8;
    break;

  case HSV :
    for (l=length; l; l--)
    {
      dicom_hsv(s[0],s[1],s[2],d);
      s+=3;
      d+=3;
    }
    break;

  case ARGB :
    for (l=length; l; l--)
      if (*s)
      {
	*d++=dicom_clut(single->clut,  *s)>>8;
	*d++=dicom_clut(single->clut+1,*s)>>8;
	*d++=dicom_clut(single->clut+2,*s)>>8;
	s+=4;
      }
      else
      {
        s++;
        *d++=*s++>>8;
        *d++=*s++>>8;
        *d++=*s++>>8;
      }
    break;

  case CMYK :
    for (l=length; l; l--)
    {
      *d++=(0xFFFF-*s++)>>8;
      *d++=(0xFFFF-*s++)>>8;
      *d++=(0xFFFF-*s++)>>8;
      s++;
    }
    break;

  default :
    break;
  }

  return &image;
}

/********
 * clut *
 ********/

static U16 dicom_clut(const CLUT *clut,U16 i)
{
  if (i<=clut->threshold.u16)
    return clut->data.u16[0];

  i-=clut->threshold.u16;

  if (i>=clut->size-1)
    return clut->data.u16[clut->size-1];

  return clut->data.u16[i];
}
