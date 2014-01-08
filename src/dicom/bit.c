/*************************
 * libdicom by Tony Voet *
 *************************/
/*
 * $Id: bit.c,v 1.2 2005/07/29 12:42:18 oph Exp $
 */

#include "dicom.h"

union
{
  U32	*u32;
  U16	*u16;
  U8	*u8;
}
source;

U32	cache32;
U16	cache16;
U8	cache8;

int left;

/*******
 * bit *
 *******/

void dicom_bit(void *data)
{
  dicom_log(DICOM_DEBUG,"dicom_bit()");

  source.u32=data;
  left=0;
}

/**********
 * 8 skip *
 **********/

void dicom_8_skip(int bit)
{
  if (!bit)
    return;

  if (bit<left)
  {
    cache8<<=bit;
    left-=bit;
  }
  else
  {
    bit-=left;

    cache8=*source.u8++;
    left=8;

    if (bit)
      dicom_8_skip(bit);
  }
}

/***********
 * 16 skip *
 ***********/

void dicom_16_skip(int bit)
{
  if (!bit)
    return;

  if (bit<left)
  {
    cache16<<=bit;
    left-=bit;
  }
  else
  {
    bit-=left;

    cache16=*source.u16++;
    left=16;

    if (bit)
      dicom_16_skip(bit);
  }
}

/***********
 * 32 skip *
 ***********/

void dicom_32_skip(int bit)
{
  if (!bit)
    return;

  if (bit<left)
  {
    cache32<<=bit;
    left-=bit;
  }
  else
  {
    bit-=left;

    cache32=*source.u32++;
    left=32;

    if (bit)
      dicom_32_skip(bit);
  }
}

/**********
 * 8 read *
 **********/

U32 dicom_8_read(int bit)
{
  U32 result;

  if (!bit)
    return 0;

  if (bit<left)
  {
    result=cache8>>(8-bit);
    cache8<<=bit;
    left-=bit;
  }
  else
  {
    result=cache8>>(8-left);
    bit-=left;

    cache8=*source.u8++;
    left=8;

    if (!bit)
      return result;

    result<<=bit;
    result|=dicom_8_read(bit);
  }

  return result;
}

/***********
 * 16 read *
 ***********/

U32 dicom_16_read(int bit)
{
  U32 result;

  if (!bit)
    return 0;

  if (bit<left)
  {
    result=cache16>>(16-bit);
    cache16<<=bit;
    left-=bit;
  }
  else
  {
    result=cache16>>(16-left);
    bit-=left;

    cache16=*source.u16++;
    left=16;

    if (!bit)
      return result;

    result<<=bit;
    result|=dicom_16_read(bit);
  }

  return result;
}

/***********
 * 32 read *
 ***********/

U32 dicom_32_read(int bit)
{
  U32 result;

  if (!bit)
    return 0;

  if (bit<left)
  {
    result=cache32>>(32-bit);
    cache32<<=bit;
    left-=bit;
  }
  else
  {
    result=cache32>>(32-left);
    bit-=left;

    cache32=*source.u32++;
    left=32;

    if (!bit)
      return result;

    result<<=bit;
    result|=dicom_32_read(bit);
  }

  return result;
}

/* eNlf: BEGIN - support for 12bit unpacking */
/*************
 * 12 unpack *
 *************/

/* 2 pix 12bit =     [0xABCDEF]      */
/* 2 pix 16bit = [0x0ABD] + [0x0FCE] */
U16 mdc_dicom_12_unpack(int pix)
{
  U16 result;
  U8  b0, b1, b2;
  switch (pix) {
    case 1: /* ABD-part (1st pix) */
        b0 = *source.u8++;
        b1 = *source.u8;
        result = ((b0 >> 4) << 8) + ((b0 & 0x0f) << 4) + (b1 & 0x0f);
                      /* A */          /* B */            /* D */
        break;
    case 2: /* FCE-part (2nd pix) */
        b1 = *source.u8++;
        b2 = *source.u8++;
        result = ((b2 & 0x0f) << 8) + ((b1 >> 4) << 4) + (b2 >> 4);
                      /* F */          /* C */            /* E */
        break;
    default:
        result = 0;
  }

  return result;
}
/* eNlf: END   - support for 12 bits unpacking */
