/*************************
 * libdicom by Tony Voet *
 *************************/
/*
 * $Id: process.c,v 1.2 2005/07/29 12:42:18 oph Exp $
 */

#include <stdlib.h>
#include "dicom.h"

/*******
 * max *
 *******/

void dicom_max(IMAGE *image)
{
  U32	length,l;
  U16	min,max,*pixel;

  dicom_log(DICOM_DEBUG,"dicom_max()");

  if (!image)
  {
    dicom_log(WARNING,"No image given");
    return;
  }

  if (image->rgb)
  {
    dicom_log(WARNING,"Color image");
    return;
  }

  length=image->frames*image->w*image->h;
  pixel=image->data.gray;

  min=*pixel;
  max=min;

  for (l=length; l; l--,pixel++)
  {
    if (*pixel<min)
    {
      min=*pixel;
      continue;
    }
    if (*pixel>max)
      max=*pixel;
  }

  if (min==max)
    return;

  if (min==0)
    if (max==0xFFFFU)
      return;

  pixel=image->data.gray;

  for (l=length; l; l--, pixel++)
    *pixel=0xFFFFUL*(*pixel-min)/(max-min);
}

/**********
 * invert *
 **********/

void dicom_invert(IMAGE *image)
{
  U32	l;
  U16	*pixel;

  dicom_log(DICOM_DEBUG,"dicom_invert()");

  if (!image)
  {
    dicom_log(WARNING,"No image given");
    return;
  }

  if (image->rgb)
  {
    dicom_log(WARNING,"Color image");
    return;
  }

  pixel=image->data.gray;

  for (l=image->frames*image->w*image->h; l; l--, pixel++)
    *pixel=0xFFFFU-*pixel;
}

/*******
 * voi *
 *******/

void dicom_voi(IMAGE *image,U16 min,U16 max)
{
  U32	l;
  U16	*pixel;

  dicom_log(DICOM_DEBUG,"dicom_voi()");

  if (min==0)
    if (max==0xFFFFU)
      return;

  if (!image)
  {
    dicom_log(WARNING,"No image given");
    return;
  }

  if (image->rgb)
  {
    dicom_log(WARNING,"Color image");
    return;
  }

  pixel=image->data.gray;

  for (l=image->frames*image->w*image->h; l; l--,pixel++)
  {
    if (*pixel<=min)
    {
      *pixel=0;
      continue;
    }

    if (*pixel>=max)
    {
      *pixel=0xFFFFU;
      continue;
    }

    *pixel=0xFFFFUL*(*pixel-min)/(max-min);
  }
}

/********
 * gray *
 ********/

void dicom_gray(IMAGE *image)
{
  U32	length,l;
  U16	*target;
  U8	*source;

  dicom_log(DICOM_DEBUG,"dicom_gray()");

  if (!image)
  {
    dicom_log(WARNING,"No image given");
    return;
  }

  if (!image->rgb)
    return;

  length=image->frames*image->w*image->h;

  source=image->data.rgb;
  target=image->data.gray;

  for (l=length; l; l--,source+=3)
    *target++=77UL*source[0]+151UL*source[1]+29UL*source[2];

  image->rgb=0;

  target=realloc(image->data.gray,2*length);
  if (!target)
    dicom_log(WARNING,"Error reallocating memory");
  else
    image->data.gray=target;

  dicom_max(image);

}

/* eNlf: BEGIN - add support for indexed color, use external function */
/***********
 *  color  *
 ***********/

void dicom_color(IMAGE *image, U8 *palette, U8 dither, char *(*reduce)())
{
  U32   l, size, length;
  U16   *target16;
  U8    *dest;

  dicom_log(DICOM_DEBUG,"dicom_color()");

  if (!image)
  {
    dicom_log(WARNING,"No image given");
    return;
  }

  if (!image->rgb)
  {
    dicom_log(WARNING,"No RGB image given");
    return;
  }

  if (reduce == NULL)
  {
    dicom_log(WARNING,"Missing color quantization function");
    return;
  }

  size = image->w * image->h;
  length = size * image->frames;

  /* work with 8-bits values */
  dest = malloc(length);
  if (!dest)
    dicom_log(WARNING,"Error allocation 8bits memory");

  /* reduce RGB to indexed, but for all frames at once */
  /* otherwise different palette for each image        */ 
  reduce(image->data.rgb,dest,image->w,image->h*image->frames,palette,dither);

  image->rgb=0;

  /* translate to 16-bits values */
  target16=realloc(image->data.gray,2*length);
  if (!target16)
    dicom_log(WARNING,"Error reallocating memory");
  for (l=0; l<length; l++) target16[l] = (U16)dest[l];

  eNlfSafeFree(dest);

  image->data.gray=target16;

}
/* eNlf: END   - add support for indexed color, use external function */

/*******
 * hsv *
 *******/

void dicom_hsv(U16 h,U16 s,U16 v,U8 *rgb)
{
  float	hue,saturation,f;
  int	i;
  U8	value,m,n;

  hue=h*6.0/65536.0;
  saturation=s/65535.0;
  value=v>>8;

  i=hue;

  f=hue-i;
  if (!(i&1))
    f=1.0-f;

  m=value*(1.0-saturation);
  n=value*(1.0-saturation*f);

  switch(i)
  {
  case 0 :
    rgb[0]=value;
    rgb[1]=n;
    rgb[2]=m;
    break;

  case 1 :
    rgb[0]=n;
    rgb[1]=value;
    rgb[2]=m;
    break;

  case 2 :
    rgb[0]=m;
    rgb[1]=value;
    rgb[2]=n;
    break;

  case 3 :
    rgb[0]=m;
    rgb[1]=n;
    rgb[2]=value;
    break;

  case 4 :
    rgb[0]=n;
    rgb[1]=m;
    rgb[2]=value;
    break;

  case 5 :
    rgb[0]=value;
    rgb[1]=m;
    rgb[2]=n;
  }
}

/*********
 * merge *
 *********/

IMAGE *dicom_merge(const IMAGE *anatomic,const IMAGE *parametric,U16 saturation)
{
  IMAGE	*zoom,*merge;
  U16	bar,*value,*hue,frame,x,y;
  U8	*target;

  dicom_log(DICOM_DEBUG,"dicom_merge()");

  if (!anatomic || !parametric)
  {
    dicom_log(ERROR,"Image missing");
    return 0L;
  }

  if (anatomic->rgb || parametric->rgb)
  {
    dicom_log(ERROR,"Wrong image type");
    return 0L;
  }

  if (anatomic->frames!=parametric->frames)
  {
    dicom_log(ERROR,"Wrong number of frames");
    return 0L;
  }

  zoom=dicom_zoom(parametric,anatomic->w,anatomic->h,-1);
  if (!zoom)
    return 0L;

  bar=anatomic->w>>5;

  merge=dicom_new(-1,anatomic->frames,anatomic->w+(bar<<1),anatomic->h);
  if (!merge)
  {
    dicom_free(zoom,1);
    return 0L;
  }

  value=anatomic->data.gray;
  hue=zoom->data.gray;
  target=merge->data.rgb;

  for (frame=anatomic->frames; frame; frame--)
    for (y=0; y<anatomic->h; y++)
    {
      for (x=anatomic->w; x; x--)
      {
	dicom_hsv(2UL*(0xFFFFU-*hue)/3U,*hue?saturation:0,*value,target);

	value++;
	hue++;
	target+=3;
      }

      for (x=3*bar; x; x--)
	*target++=0;

      for (x=bar; x; x--)
      {
	dicom_hsv(0xAAAAUL*y/(anatomic->h-1),saturation,0xFFFFU,target);
	target+=3;
      }
    }

  dicom_free(zoom,1);

  return merge;
}
