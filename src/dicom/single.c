/*************************
 * libdicom by Tony Voet *
 *************************/
/*
 * $Id: single.c,v 1.2 2005/07/29 12:42:18 oph Exp $
 */

#include <stdlib.h>
#include <string.h>
#include "dicom.h"

#define MDC_ENCAP_OFFSET_TABEL 0               /* 0/1 using offset table */

static S32 dicom_pixel(const ELEMENT *);

static SINGLE single;

/**********
 * single *
 **********/

SINGLE *dicom_single(void)
{
  ELEMENT	*e;
  S32		length;
  U32		i, f;
  char		*interpretation[]=
  {
    "MONOCHROME2",
    "MONOCHROME1",
    "PALETTE COLOR",
    "RGB",
    "HSV",
    "ARGB",
    "CMYK",
    "UNKNOWN"
  };

  dicom_log(DICOM_DEBUG,"dicom_single()");

  memset(&single,0,sizeof(SINGLE));
  single.frames=1;
  single.samples=1;

  for (;;)
  {
    e=dicom_element();
    if (!e)
      break;

    if (mdc_dicom_skip_sequence(e)) {
      if (dicom_skip()) break;
      continue;
    }

    if (e->group==0x0028)
    {
      if (e->element==0x0002)
      {
        if (dicom_load(US))
          break;
        single.samples=*e->value.US;
        eNlfSafeFree(e->value.US);
        continue;
      }

      if (e->element==0x0004)
      {
        if (dicom_load(CS))
          break;
        dicom_clean();

        for (single.photometric=0; single.photometric<UNKNOWN;
          single.photometric++)
          if ( !strncmp(*e->value.CS,interpretation[single.photometric],
            strlen(interpretation[single.photometric])) )
            break;

        if (single.photometric==UNKNOWN)
          dicom_log(WARNING,"Unknown PhotometricInterpretation");

        eNlfSafeFree(e->value.CS);
        continue;
      }

      if (e->element==0x0006)
      {
        if (dicom_load(US))
          break;
        single.planar=*e->value.US;
        eNlfSafeFree(e->value.US);
        continue;
      }

      if (e->element==0x0008)
      {
        if (dicom_load(IS))
          break;
        dicom_clean();
        single.frames=atoi(*e->value.IS);
        eNlfSafeFree(e->value.IS);
        continue;
      }

      if (e->element==0x0010)
      {
        if (dicom_load(US))
          break;
        single.h=*e->value.US;
        eNlfSafeFree(e->value.US);
        continue;
      }

      if (e->element==0x0011)
      {
        if (dicom_load(US))
          break;
        single.w=*e->value.US;
        eNlfSafeFree(e->value.US);
        continue;
      }

      if (e->element==0x0100)
      {
        if (dicom_load(US))
          break;
        single.alloc=*e->value.US;
        eNlfSafeFree(e->value.US);
        continue;
      }

      if (e->element==0x0101)
      {
        if (dicom_load(US))
          break;
        single.bit=*e->value.US;
        eNlfSafeFree(e->value.US);
        continue;
      }

      if (e->element==0x0102)
      {
        if (dicom_load(US))
          break;
        single.high=*e->value.US;
        eNlfSafeFree(e->value.US);
        if ((dicom_workaround & MDC_FIX_EZDICOM) && (single.high == 0)) {
          dicom_log(WARNING,"Wrong ezDICOM high bit value (fixed)");
          single.high = (single.bit > 0) ? single.bit - 1 : 15;
        }
        continue;
      }

      if (e->element==0x0103)
      {
        if (dicom_load(US))
          break;
        single.sign=*e->value.US;
        eNlfSafeFree(e->value.US);
        continue;
      }

      if (0x1101<=e->element && e->element<=0x1103)
      {
        if (dicom_load(US))
          break;

	if (e->vm!=3)
	  dicom_log(WARNING,"Wrong VM for PaletteColorLookupTableDescriptor");
	else
	{
	  i=e->element-0x1101;
	  single.clut[i].size=e->value.US[0];
	  single.clut[i].threshold.u16=e->value.US[1];
	  single.clut[i].bit=e->value.US[2];
	}

        eNlfSafeFree(e->value.US);
        continue;
      }

      if (0x1201<=e->element && e->element<=0x1203)
      {
        if (dicom_load(US))
          break;
	single.clut[e->element-0x1201].data.u16=e->value.US;
        continue;
      }
    }

    if (!(e->group&1)) 
    {
      if ((0x7F00<=e->group && e->group<0x7FFF) && (e->element==0x0010))
      {
        unsigned frames, width, height, pixel;

        frames= (unsigned) single.frames;
        width = (unsigned) single.w; 
        height= (unsigned) single.h;
        pixel = (unsigned) single.samples*single.alloc>>3;

        if (e->length!=0xFFFFFFFF)
	{ /* pixel data, not encapsulated */

          /* first fix bad VR values, confusing data endian swap */
          switch (e->vr)
          {
            case OB:
              if (single.alloc==16)
              {
                dicom_log(WARNING,"Incorrect OB value representation (fixed)");
                e->vr=OW;

                /* workaround for Amira 3.0 pixel data length bug     */
                /* http://www.amiravis.com/resources/Patch30-13-dicom */
                if (e->length == 2 * frames * width * height * pixel) {
                  dicom_log(WARNING,"Amira 3.0 pixel data length bug (fixed)");
                  e->length /= 2;
                }
              }
              break;
            case OW:
              if (single.alloc==8)
              {
                dicom_log(WARNING,"Incorrect OW value representation (fixed)");
                e->vr=OB;
              }
              break;
            default:
              break;
          }
         

	  length=dicom_pixel(e);
	  if (length<0)
	    break;

          if (length!= frames * width * height * pixel)
            dicom_log(WARNING,"Incorrect PixelData length");

          return &single;
	}
        else if (e->length == 0xFFFFFFFF)
        { /* encapsulated data */
          U8 *data;
#if MDC_ENCAP_OFFSET_TABLE
          U32 *offset=NULL, begin=0;
#endif
          /* skip offset table */
          e=dicom_element(); if (!e) break;
          if (e->vm && e->length != 0)
          { 
            /* a  value present */
            if (e->length != frames * 4L) break; /* get out, bad offset table */
#if MDC_ENCAP_OFFSET_TABLE
            dicom_load(UL);
            offset = e->value.UL;
            begin = mdc_dicom_ftell();
#else
            dicom_skip();
#endif
          }

          /* allocate memory for all frames, memset for sure          */
          /* eNlf: - allocate an extra 4 bytes, otherwise the bit.c   */
          /* eNlf: routines like source.u++ go beyond the boundaries  */
          /* eNlf: - memset the allocated buffer for sure             */
          data = (U8*)malloc(width*height*pixel*frames+4);
          if (!data)
          {
            dicom_log(ERROR,"Out of memory");
            return 0L;
          }
          memset(data,0,width*height*pixel*frames+4);

          single.data = data; 

          /* retrieve all frames and decompress */
          for (f=0; f<frames; f++)
          {
#if MDC_ENCAP_OFFSET_TABLE
             mdc_dicom_fseek(offset[f] + begin,SEEK_SET);
#endif
             e=dicom_element(); if (!e) break;
             e->vr=OB;
             e->value.OB = data + f*width*height*pixel;

             if (mdc_dicom_decompress(&single,e))
             {
               dicom_log(ERROR,"Decompression failed");
               dicom_single_free();
               return 0L;
             }
          }

#if MDC_ENCAP_OFFSET_TABLE
          eNlfSafeFree(offset);
#endif

          return &single;

        }
      }
    }

    if (dicom_skip())
      break;
  }

  dicom_single_free();

  return 0L;
}

/***************
 * single free *
 ***************/

void dicom_single_free(void)
{
  int i;

  dicom_log(DICOM_DEBUG,"dicom_single_free()");

  for (i=0; i<3; i++)
    eNlfSafeFree(single.clut[i].data.u16);

  eNlfSafeFree(single.data);

  memset(&single,0,sizeof(SINGLE));
}

/*********
 * pixel *
 *********/

static S32 dicom_pixel(const ELEMENT *e)
{
  U16 magic=0x1234;
  int error;

  dicom_log(DICOM_DEBUG,"dicom_pixel()");

  if (e->length!=0xFFFFFFFF)
  {
    if (single.alloc==16) {
      error=dicom_load(OW);
    }else if (single.alloc==12) {
      if ( *((U8*)&magic)==0x12 ) mdc_dicom_switch_endian(); 
      error=dicom_load(OW);
      if ( *((U8*)&magic)==0x12 ) mdc_dicom_switch_endian();
    }else{
      error=dicom_load(OB);
    }

    if (error)
      return -1;

    single.data=e->value.OW;

    return e->length;
  }

  if (dicom_skip())
    return -2;

  dicom_log(EMERGENCY,"Encapsulated PixelData is not implemented yet");

  return -3;
}
