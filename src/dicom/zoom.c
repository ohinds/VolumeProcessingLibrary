/*************************
 * libdicom by Tony Voet *
 *************************/
/*
 * $Id: zoom.c,v 1.2 2005/07/29 12:42:18 oph Exp $
 */

#include <string.h>
#include "dicom.h"

static void dicom_zoom_gray(const IMAGE *,U16 *,U16 *);
static void dicom_zoom_rgb(const IMAGE *,U8 *,U8 *);

static void dicom_hyper_gray(const IMAGE *,U16 *,U16 *);
static void dicom_hyper_rgb(const IMAGE *,U8 *,U8 *);

static IMAGE *zoom;

/********
 * zoom *
 ********/

IMAGE *dicom_zoom(const IMAGE *image,int w,int h,int hyper)
{
  U16	frame;
  int	size;
  void	*source,*target;

  dicom_log(DICOM_DEBUG,"dicom_zoom()");

  if (!image)
  {
    dicom_log(ERROR,"No image given");
    return 0L;
  }

  zoom=dicom_new(image->rgb,image->frames,w,h);
  if (!zoom)
    return 0L;

  if (zoom->rgb)
    size=3;
  else
    size=2;

  if (w==image->w && h==image->h)
  {
    memcpy(zoom->data.rgb,image->data.rgb,(unsigned)zoom->frames*
                                          (unsigned)(w*h*size));
    return zoom;
  }

  source=image->data.rgb;
  target=zoom->data.rgb;

  for (frame=zoom->frames; frame; frame--)
  {
    if (!hyper || (w<image->w && h<image->h))
      if (zoom->rgb)
        dicom_zoom_rgb(image,source,target);
      else
        dicom_zoom_gray(image,source,target);
    else
      if (zoom->rgb)
        dicom_hyper_rgb(image,source,target);
      else
        dicom_hyper_gray(image,source,target);

    source=(U8*)source+image->w*image->h*size;
    target=(U8*)target+w*h*size;
  }

  return zoom;
}

/*************
 * zoom gray *
 *************/

static void dicom_zoom_gray(const IMAGE *image,U16 *source,U16 *target)
{
  float	x,y,sx,sy;
  U16	*line;

  dicom_log(DICOM_DEBUG,"dicom_zoom_gray()");

  sx=(float) image->w/zoom->w;
  sy=(float) image->h/zoom->h;

  for (y=sy/2.0; y<image->h; y+=sy)
  {
    line=source+image->w*(int)y;
    for (x=sx/2.0; x<image->w; x+=sx)
      *target++=line[(int)x];
  }
}

/************
 * zoom rgb *
 ************/

static void dicom_zoom_rgb(const IMAGE *image,U8 *source,U8 *target)
{
  float	x,y,sx,sy;
  int	i;
  U8	*line;

  dicom_log(DICOM_DEBUG,"dicom_zoom_rgb()");

  sx=(float) image->w/zoom->w;
  sy=(float) image->h/zoom->h;

  for (y=sy/2.0; y<image->h; y+=sy)
  {
    line=source+3*image->w*(int)y;
    for (x=sx/2.0; x<image->w; x+=sx)
    {
      i=3*(int)x;
      *target++=line[i];
      *target++=line[i+1];
      *target++=line[i+2];
    }
  }
}

/**************
 * hyper gray *
 **************/

static void dicom_hyper_gray(const IMAGE *image,U16 *source,U16 *target)
{
  float	x,y,sx,sy,dx,dy;
  int	ix,iy;
  U16	*line,*next;

  dicom_log(DICOM_DEBUG,"dicom_hyper_gray()");

  sx=(float) image->w/zoom->w;
  sy=(float) image->h/zoom->h;

  for (y=sy/2.0; y<image->h; y+=sy)
  {
    iy=(int)(y+0.5)-1;

    line=source+image->w*iy;
    next=line+image->w;

    for (x=sx/2.0; x<image->w; x+=sx)
    {
      ix=(int)(x+0.5)-1;

      dx=x-ix-0.5;
      dy=y-iy-0.5;

      if (x<0.5)
        dx=1.0;
      if (image->w-0.5<x)
        dx=0.0;
      if (y<0.5)
        dy=1.0;
      if (image->h-0.5<y)
        dy=0.0;

      *target++= (1.0-dx)*(1.0-dy)*line[ix] + dx*(1.0-dy)*line[ix+1] +
        (1.0-dx)*dy*next[ix] + dx*dy*next[ix+1] + 0.5;
    }
  }
}

/*************
 * hyper rgb *
 *************/

static void dicom_hyper_rgb(const IMAGE *image,U8 *source,U8 *target)
{
  float	x,y,sx,sy,dx,dy;
  int	ix,iy,i;
  U8	*line,*next;

  dicom_log(DICOM_DEBUG,"dicom_hyper_rgb()");

  sx=(float) image->w/zoom->w;
  sy=(float) image->h/zoom->h;

  for (y=sy/2.0; y<image->h; y+=sy)
  {
    iy=(int)(y+0.5)-1;

    line=source+3*image->w*iy;
    next=line+3*image->w;

    for (x=sx/2.0; x<image->w; x+=sx)
    {
      ix=(int)(x+0.5)-1;

      dx=x-ix-0.5;
      dy=y-iy-0.5;

      if (x<0.5)
        dx=1.0;
      if (image->w-0.5<x)
        dx=0.0;
      if (y<0.5)
        dy=1.0;
      if (image->h-0.5<y)
        dy=0.0;

      ix*=3;

      for (i=3; i; i--)
      {
        *target++= (1.0-dx)*(1.0-dy)*line[ix] + dx*(1.0-dy)*line[ix+4] +
          (1.0-dx)*dy*next[ix] + dx*dy*next[ix+4] + 0.5;

        ix++;
      }
    }
  }
}
