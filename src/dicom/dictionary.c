/*************************
 * libdicom by Tony Voet *
 *************************/
/* 
 * $Id: dictionary.c,v 1.2 2005/07/29 12:42:18 oph Exp $
 */

#include "dicom.h"

/*********
 * query *
 *********/

DICTIONARY *dicom_query(ELEMENT *element)
{
  static DICTIONARY data[]=
  {
    #include "dictionary.data"
  };

  dicom_log(DICOM_DEBUG,"dicom_query()");

  if (!element)
  {
    dicom_log(ERROR,"No element given");
    return 0L;
  }

  return dicom_private(data,element);
}

/***********
 * private *
 ***********/

DICTIONARY *dicom_private(DICTIONARY *data,ELEMENT *e)
{
  static DICTIONARY *d;

  dicom_log(DICOM_DEBUG,"dicom_private()");

  if (!data)
  {
    dicom_log(ERROR,"No dictionary given");
    return 0L;
  }

  if (!e)
  {
    dicom_log(ERROR,"No element given");
    return 0L;
  }

  for (d=data; d->group!=0xFFFF; d++)
  {
    if (e->group<d->group)
      continue;

    if (e->group>d->group_last)
      continue;

    switch(d->group_match)
    {
    case ANY :
      break;

    case EVEN :
      if (e->group&1)
        continue;
      break;

    case ODD :
      if (!(e->group&1))
        continue;
    }

    if (e->element<d->element)
      continue;

    if (e->element>d->element_last)
      continue;

    switch(d->element_match)
    {
    case ANY :
      break;

    case EVEN :
      if (e->element&1)
        continue;
      break;

    case ODD :
      if (!(e->element&1))
        continue;
    }

    break;
  }

  return d;
}
