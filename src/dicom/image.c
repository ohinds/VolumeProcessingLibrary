/*************************
 * libdicom by Tony Voet *
 *************************/
/* 
 * $Id: image.c,v 1.3 2005/07/29 17:09:54 oph Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dicom.h"
/* eNlf: BEGIN - comment out unwanted stuff */
/*
#include "jpeglib.h"
*/
/* eNlf: END   - comment out unwanted stuff */

/*******
 * new *
 *******/

IMAGE *dicom_new(int rgb,U16 frames,U16 w,U16 h)
{
  IMAGE *image;

  dicom_log(DICOM_DEBUG,"dicom_new()");

  image=malloc(sizeof(IMAGE));
  if (!image)
  {
    dicom_log(ERROR,"Out of memory");
    return 0L;
  }

  image->rgb=rgb;
  image->frames=frames;
  image->w=w;
  image->h=h;

  if (rgb)
    image->data.rgb=malloc((unsigned)(frames*w*h)*3U);
  else
    image->data.gray=malloc((unsigned)(frames*w*h)*2U);

  if (!image->data.rgb)
  {
    dicom_log(ERROR,"Out of memory");

    eNlfSafeFree(image);
    return 0L;
  }

  return image;
}

/********
 * read *
 ********/

/* oph: added numToRead parameter, controlling the number of frames to read */
int dicom_read(const char *file,IMAGE **image,int *images,int numToRead,
	       int parametric)
{
  SINGLE	*single;
  IMAGE		*new,*tmp;

  dicom_log(DICOM_DEBUG,"dicom_read()");

  if (!file)
  {
    dicom_log(ERROR,"No file given");
    return -1;
  }

  if (!image || !images)
  {
    dicom_log(ERROR,"Argument missing");
    return -2;
  }

  if (dicom_open(file))
    return -3;

  for (*image=0L,*images=0;*images < numToRead;)
  {
    single=dicom_single();
    if (!single)
      break;

    new=dicom_transform(single,parametric);
    if (new)
    {
      if (*image) 
        tmp=realloc(*image,(*images+1)*sizeof(IMAGE));
      else
        tmp=malloc(sizeof(IMAGE));

      if (!tmp)
      {
        dicom_log(ERROR,"Error reallocating memory");
        eNlfSafeFree(new->data.rgb);
      }
      else
      {
        *image=tmp;
        memcpy(*image+*images,new,sizeof(IMAGE));
        (*images)++;
      }
    }

    dicom_single_free();
  }

  if (*images==0)
  {
    dicom_log(ERROR,"No images found");
    /* eNlf: BEGIN -- changes for integration in MedCon */
    /* eNlf: END   -- changes for integration in MedCon */
    return -4;
  }
  dicom_close();

  return 0;
}

/********
 * free *
 ********/

void dicom_free(IMAGE *image,int images)
{
  int i;

  dicom_log(DICOM_DEBUG,"dicom_free()");

  if (!image)
    return;

  for (i=0; i<images; i++)
    eNlfSafeFree(image[i].data.rgb);

  eNlfSafeFree(image);
}

/*********
 * write *
 *********/

int dicom_write(const char *file,const IMAGE *image)
{
  dicom_log(DICOM_DEBUG,"dicom_write()");

  if (!file)
  {
    dicom_log(ERROR,"No file given");
    return -1;
  }

  if (!image)
  {
    dicom_log(ERROR,"No image given");
    return -2;
  }

  dicom_log(EMERGENCY,"DICOM write is not implemented yet");

  return -3;
}
/* eNlf: BEGIN - comment out unwanted stuff */
/***************
 * write ascii *
 ***************/
/*
int dicom_write_ascii(const char *file,const IMAGE *image,int width)
{
  static char gray[]=
    "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

  FILE	*stream;
  IMAGE	*zoom;
  U16	frame,x,y,*pixel;

  dicom_log(DICOM_DEBUG,"dicom_write_ascii()");

  if (!image)
  {
    dicom_log(ERROR,"No image given");
    return -1;
  }

  if (!file)
    stream=stdout;
  else
  {
    stream=fopen(file,"wb");
    if (!stream)
    {
      dicom_log(ERROR,"Unable to create ascii file");
      return -2;
    }
  }

  zoom=dicom_zoom(image,width,width*image->h/(image->w<<1),-1);
  if (!zoom)
    return -3;

  dicom_gray(zoom);
  dicom_max(zoom);

  pixel=zoom->data.gray;

  for (frame=zoom->frames; frame; frame--)
  {
    for (y=zoom->h; y; y--)
    {
      for (x=zoom->w; x; x--,pixel++)
	putc(gray[(int) 69**pixel/0xFFFF],stream);
      puts("");
    }
    puts("");
  }

  dicom_free(zoom,1);

  return 0;
}
*/

/**************
 * write jpeg *
 **************/
/*
int dicom_write_jpeg(const char *file,const IMAGE *image,int quality)
{
  struct jpeg_compress_struct	cinfo;
  struct jpeg_error_mgr		jerr;
  JSAMPROW			line,target;
  FILE				*stream;
  U16				*source,l;

  dicom_log(DICOM_DEBUG,"dicom_write_jpeg()");

  if (!file)
  {
    dicom_log(ERROR,"No file given");
    return -1;
  }

  if (!image)
  {
    dicom_log(ERROR,"No image given");
    return -2;
  }

  if (!image->rgb)
  {
    line=malloc(image->w*2);
    if (!line)
    {
      dicom_log(ERROR,"Out of memory");
      return -3;
    }
  }

  stream=fopen(file,"wb");
  if (!stream)
  {
    dicom_log(ERROR,"Unable to create jpeg file");
    return -4;
  }

  cinfo.err=jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo,stream);

  cinfo.image_width=image->w;
  cinfo.image_height=image->h*image->frames;

  if (image->rgb)
  {
    cinfo.input_components=3;
    cinfo.in_color_space=JCS_RGB;
  }
  else
  {
    cinfo.input_components=1;
    cinfo.in_color_space=JCS_GRAYSCALE;
  }

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo,quality,-1);
  jpeg_start_compress(&cinfo,-1);

  while (cinfo.next_scanline<cinfo.image_height)
  {
    if (image->rgb)
      line=image->data.rgb+cinfo.next_scanline*image->w*3;
    else
    {
      source=image->data.gray+cinfo.next_scanline*image->w;
      target=line;

      for (l=image->w; l; l--)
        *target++=*source++>>8;
    }

    jpeg_write_scanlines(&cinfo,&line,1);
  }

  if (!image->rgb)
    eNlfSafeFree(line);

  jpeg_finish_compress(&cinfo);
  fclose(stream);
  jpeg_destroy_compress(&cinfo);

  return 0;
}
*/
/*************
 * write eps *
 *************/
/*
int dicom_write_eps(const char *file,const IMAGE *image)
{
  dicom_log(DICOM_DEBUG,"dicom_write_eps()");

  if (!file)
  {
    dicom_log(ERROR,"No file given");
    return -1;
  }

  if (!image)
  {
    dicom_log(ERROR,"No image given");
    return -2;
  }

  dicom_log(EMERGENCY,"DICOM write EPS is not implemented yet");

  return -3;
}
*/
/*eNlf: END  - comment out unwanted stuff */

