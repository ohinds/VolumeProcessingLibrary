/*************************
 * libdicom by Tony Voet *
 *************************/
/*
 * $Id: basic.c,v 1.3 2006/05/09 15:00:36 oph Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dicom.h"

char dicom_version[]="libdicom 0.31",**dicom_transfer_syntax=0L;

WORKAROUND dicom_workaround;

static void	dicom_transfer(void);
static void	dicom_vr(void);
static void	dicom_encapsulated(int);
static void	dicom_sequence(int);
static void	dicom_endian(void);
static int	dicom_vm(void);

#if MDC_DICOM_DICOM_DEBUG
static void     mdc_dicom_debug_tag(void);
#endif
static void	mdc_dicom_endian(void);
	
static ELEMENT	element;
static FILE	*stream=0L;
static long	position;
static int	meta;
static enum
{
  LITTLE=1,
  BIG=2,
  IMPLICIT=4,
  EXPLICIT=8,
  COMPRESSED_UNKNOWN=16,
  COMPRESSED_LOSSLESS = 32,
  COMPRESSED_LOSSLY = 64,
  COMPRESSED_RLE = 128
}
syntax,endian,filesyntax,pixelsyntax,encapsyntax;

#if MEDCON_INTEGRATED
/* eNlf: routine for setting the stream from outside the library    */
/* eNlf: in MedCon this library doesn't have to open or close steam */         
/********
 * init *
 ********/
void dicom_init(FILE *fp)
{
   stream = fp;

   dicom_workaround = 0;
}
#endif

/********
 * open *
 ********/

int dicom_open(const char *file)
{
  U16	magic=0x1234;
  char  vr[2];
#if MEDCON_INTEGRATED
  char  buffer[512];
#else
  char	*dot,tmp[255],buffer[512];
#endif 

  dicom_log(DICOM_DEBUG,"dicom_open()");

#if !MEDCON_INTEGRATED
  if (!file)
  {
    dicom_log(ERROR,"No file given");
    return -1;
  }

  dot=strrchr(file,'.');
  if (dot)
    if (!strcmp(dot,".gz") || !strcmp(dot,".Z"))
    {
      strcpy(tmp,"/tmp/dicom_tmp");

      sprintf(buffer,"gzip -cd %.435s > %.64s",file,tmp);
      if (system(buffer))
      {
        dicom_log(ERROR,"Unable to uncompress file");
        unlink(tmp);
        return -2;
      }

      stream=fopen(tmp,"rb");
      unlink(tmp);

      if (!stream)
      {
        dicom_log(ERROR,"Unable to open temporary file");
        return -3;
      }
    }

  if (!stream)
  {
    stream=fopen(file,"rb");
    if (!stream)
    {
      dicom_log(ERROR,"Unable to open file");
      return -4;
    }
  }
#else
  if (!stream)
  {
    dicom_log(ERROR,"Bad null stream");
    return -4;
  }
#endif

  fread(buffer,1,132,stream);
  if (dicom_check(0))
    return -5;

  if (!strncmp(buffer+128,"DICM",4))
  {
    buffer[128]=0;
    dicom_log(INFO,"Dicom preamble");
    dicom_log(INFO,buffer);

    meta=-1;
    syntax=LITTLE|EXPLICIT;

    /* watch out for LITTLE|IMPLICIT */
    fread(&element.group,2,2,stream);
    dicom_swap(&element.group,2);
    dicom_swap(&element.element,2);
    fread(vr,1,2,stream);
    element.vr=(*vr<<8)|vr[1];

    if (element.vr != UL) syntax=LITTLE|IMPLICIT;  /* weird */

    fseek(stream,132,SEEK_SET);

  }
  else
  {
    rewind(stream);

    meta=0;

    if (*buffer) {
      if (buffer[5]) {
        syntax=LITTLE|EXPLICIT;
      }else{
        syntax=LITTLE|IMPLICIT;
      }
    }else{
      if (buffer[4]) {
        syntax=BIG|EXPLICIT;
      }else{
        syntax=BIG|IMPLICIT;
      }
    }
  }

  filesyntax=syntax; pixelsyntax=syntax;

  if ( *((U8*)&magic)==0x12 )
    endian=BIG;
  else
    endian=LITTLE;

  dicom_encapsulated(-1);
  dicom_sequence(-1);

  return 0;
}

/***********
 * element *
 ***********/

ELEMENT *dicom_element(void)
{
  long	rewind;
  U16	tmp;
  char	vr[2];

  dicom_log(DICOM_DEBUG,"dicom_element()");

  if (!stream)
    return 0L;

  position=ftell(stream);

  fread(&element.group,2,2,stream);
  if (dicom_check(-1))
    return 0L;
  dicom_swap(&element.group,2);
  dicom_swap(&element.element,2);

  /* fix ezDICOM wrong transfer syntax   */
  /* MARK: 0x0800 not considered a group */
  if ((element.group == 0x0800) && (syntax & BIG)) {
    dicom_log(WARNING,"Fix ezDICOM false endian transfer syntax");
    dicom_workaround ^= MDC_FIX_EZDICOM;
    if (syntax & endian) {
      /* no previous swaps */
      mdc_dicom_switch_syntax_endian();
      dicom_swap(&element.group,2);
      dicom_swap(&element.element,2); 
    }else{
      /* undo previous swaps */
      dicom_swap(&element.group,2);
      dicom_swap(&element.element,2); 
      mdc_dicom_switch_syntax_endian();
    }
  }

  if (meta)
    if (element.group>=0x0008)
    {
      meta=0;
      dicom_transfer();

      fseek(stream,position,SEEK_SET);  
      return dicom_element();
    }

  if (syntax & IMPLICIT || element.group==0xFFFE)
  {
    dicom_vr();

    fread(&element.length,4,1,stream);
    dicom_swap(&element.length,4);
  }
  else
  {
    fread(vr,1,2,stream);
    element.vr=(*vr<<8)|vr[1];

    switch(element.vr)
    {
    case OB :
    case OW :
    case SQ :
    case UN :
    case UT :
      fseek(stream,2,SEEK_CUR);

      fread(&element.length,4,1,stream);
      dicom_swap(&element.length,4);
      break;

    default :
      fread(&tmp,2,1,stream);
      dicom_swap(&tmp,2);
      element.length=tmp;
    }
  }

  if (dicom_check(0))
    return 0L;

#if MDC_DICOM_DICOM_DEBUG
  /* show tags before further processing */
  mdc_dicom_debug_tag();
#endif

  if (element.length == 13) {
    /* fix naughty  GE tag length */
    dicom_log(WARNING,"Fix naughty GE tag length");
    element.length = 10;
  }else if (((element.length % 2) != 0) && (element.length != 0xffffffff)) {
    /* debug info for uneven tag length */
    dicom_log(WARNING,"Tag with uneven length");
  }
  
  dicom_encapsulated(0);
  dicom_sequence(0);

  if (element.group==0x0002)
    if (element.element==0x0010)
    {
      rewind=ftell(stream);
      if (dicom_load(UI))
        return 0L;
      fseek(stream,rewind,SEEK_SET);
      dicom_transfer_syntax=element.value.UI;
    }

  return &element;
}

/********
 * skip *
 ********/

int dicom_skip(void)
{
  dicom_log(DICOM_DEBUG,"dicom_skip()");

  if (!stream)
  {
    dicom_log(WARNING,"Stream closed - attempt to skip");
    return -1;
  }

  if (element.vr==SQ || element.length==0xFFFFFFFF)
    return 0;

  if (element.group==0xFFFE)
    if (!element.encapsulated)
      return 0;

  fseek(stream,(long)element.length,SEEK_CUR);

  return dicom_check(0);
}

int mdc_dicom_skip_sequence(ELEMENT *e)
{
  int answer = 0;

  if (e->sequence) {
    if (( e->sqtag.group == 0x0088) && (e->sqtag.element == 0x0200)) {
      answer = 1;
    }
  }

  return(answer);
}

/*************
 * MDC fseek *
 *************/

int mdc_dicom_fseek(U32 offset, int whence)
{
  fseek(stream,(long)offset,whence);

  return(dicom_check(0));
}

/*************
 * MDC ftell *
 *************/
U32 mdc_dicom_ftell(void)
{
  return(ftell(stream));
}

/********
 * load *
 ********/

int dicom_load(VR vr)
{
  dicom_log(DICOM_DEBUG,"dicom_load()");

  if (!stream)
  {
    dicom_log(WARNING,"Stream closed - attempt to load");
    return -1;
  }

  if (element.vr==UN)
    element.vr=vr;

  if (element.vr==SQ || element.length==0xFFFFFFFF)
    return 0;

  if (element.group==0xFFFE)
    if (!element.encapsulated)
      return 0;

  if (!element.length)
    element.value.UN=0L;
  else
  {
    /* eNlf: - allocate an extra 4 bytes, otherwise the bit.c   */
    /* eNlf: routines like source.u++ go beyond the boundaries  */
    /* eNlf: - memset the allocated buffer for sure             */
    element.value.UN=malloc(element.length + 4);
    if (!element.value.UN)
    {
      dicom_log(ERROR,"Out of memory");
      dicom_close();
      return -2;
    }
    memset(element.value.UN,0,element.length + 4);
    fread(element.value.UN,1,element.length,stream);
    if (dicom_check(0))
    {
      eNlfSafeFree(element.value.UN);
      return -3;
    }

    mdc_dicom_endian();

  }

  return dicom_vm();
}

#if MDC_DICOM_DICOM_DEBUG
/*****************
 * MDC tag debug *
 *****************/
void mdc_dicom_debug_tag(void)
{
  fprintf(stdout,"##### TAG DICOM_DEBUG %12u: (%.4X,%.4X) %c%c[%u] (%u bytes)\n"
                ,position
                ,element.group,element.element
                ,element.vr>>8,element.vr&0xFF,element.vm
                ,element.length);
}
#endif

/**************
 * MDC endian *
 **************/
/* 
 * fix endian, take care of special pixel syntax
 */
void mdc_dicom_endian(void)
{
  if ((element.group==0x7FE0) && (element.element == 0x0010)) {
    syntax=pixelsyntax;
    dicom_endian();
    syntax=filesyntax;
  }else{
    dicom_endian();
  }
}

void mdc_dicom_switch_endian(void)
{
  endian = (endian == LITTLE) ? BIG : LITTLE;
}

void mdc_dicom_switch_syntax_endian(void)
{
  syntax ^= 0x3;    /* endian in first two bits, so flip with XOR 0011 */
}

/************
 * MDC load *
 ************/
/* eNlf: BEGIN -- changes for integration in MedCon */
/*
   Routine for MedCon, at the end the tags are not handled by 
   dicom_vm() so we can pass the tag through our MdcDoTag()
   routine and get the header info we need */
int mdc_dicom_load(VR vr)
{
  dicom_log(DICOM_DEBUG,"dicom_load()");

  if (!stream)
  {
    dicom_log(WARNING,"Stream closed - attempt to load");
    return -1;
  }

  if (element.vr==UN)
    element.vr=vr;

  if (element.vr==SQ || element.length==0xFFFFFFFF)
    return 0;

  if (element.group==0xFFFE)
    if (!element.encapsulated)
      return 0;

  if (!element.length)
    element.value.UN=0L;
  else
  {
    /* eNlf: allocate an extra 4 bytes - see also dicom_load() */
    element.value.UN=malloc(element.length + 4);
    if (!element.value.UN)
    {
      dicom_log(ERROR,"Out of memory");
      dicom_close();
      return -2;
    }
    memset(element.value.UN,0,element.length + 4);
    fread(element.value.UN,1,element.length,stream);
    if (dicom_check(0))
    {
      eNlfSafeFree(element.value.UN);
      return -3;
    }

    mdc_dicom_endian();

  }

  return 0;
}
/* eNlf: END   -- changes for integration in MedCon */

/*********
 * clean *
 *********/

void dicom_clean(void)
{
  U32	i;
  char	*c;

  dicom_log(DICOM_DEBUG,"dicom_clean()");

  switch(element.vr)
  {
  case PN :
    for (i=0; i<element.vm; i++)
      for (c=element.value.PN[i]; *c; c++)
        if (*c=='^')
          *c=' ';

  case AE :
  case AS :
  case CS :
  case DA :
  case DS :
  case DT :
  case IS :
  case LO :
  case SH :
  case TM :
  case UI :
    for (i=0; i<element.vm; i++)
    {
      for (c=element.value.AE[i]; *c; c++)
        if (*c==' ' || *c=='\t')
          element.value.AE[i]++;
        else
          break;

      for (; *c; c++);
      c--;

      for (; c>=element.value.AE[i]; c--)
        if (*c==' ' || *c=='\t')
          *c=0;
        else
          break;
    }
    break;

  default:
    break;
  }
}

/*********
 * close *
 *********/

int dicom_close(void)
{
  dicom_log(DICOM_DEBUG,"dicom_close()");

  if (!stream)
    return 0;

  eNlfSafeFree(dicom_transfer_syntax);
  dicom_transfer_syntax=0L;

#if ! MEDCON_INTEGRATED 

  if (fclose(stream))
  {
    dicom_log(WARNING,"Unable to close file");
    stream=0L;
    return -1;
  }

  stream=0L;

#else
  fseek(stream,0,SEEK_SET);
#endif


  return 0;
}

/************
 * transfer *
 ************/

static void dicom_transfer(void)
{
  dicom_log(DICOM_DEBUG,"dicom_transfer()");

  if (!dicom_transfer_syntax)
  {
    dicom_log(WARNING,"No transfer syntax found");
    return;
  }

  if (strncmp(*dicom_transfer_syntax,"1.2.840.113619.5.2",18) == 0)
  {
    syntax=LITTLE|IMPLICIT;
    filesyntax=syntax; pixelsyntax=BIG|IMPLICIT;
    return;
  }
  if (strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2",17))
  {
    dicom_log(WARNING,"Transfer syntax is not DICOM");
    return;
  }

  encapsyntax = 0;
  if (!strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4",19))  /* JPEG */
  {
    if (!strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.50",22) ||
        /* baseline */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.51",22) ||
        /* extended */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.52",22) ||
        /* extended */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.53",22) ||
        /* spectral selection, non-hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.54",22) ||
        /* spectral selection, non-hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.55",22) ||
        /* full progression, non-hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.56",22) ||
        /* full progression, non-hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.59",22) ||
        /* extended, hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.60",22) ||
        /* extended, hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.61",22) ||
        /* spectral selection, hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.62",22) ||
        /* spectral selection, hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.63",22) ||
        /* full progression, hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.64",22) )
        /* full progression, hierarchical */
    {
      encapsyntax = COMPRESSED_LOSSLY;
      return;
    }
    else
    if (!strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.57",22) ||
        /* lossless, non-hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.58",22) ||
        /* lossless, non-hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.65",22) ||
        /* lossless, hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.66",22) ||
        /* lossless, hierarchical */
        !strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.4.70",22) )
        /* lossless, hierarchical,, first order prediction */
    {
      encapsyntax = COMPRESSED_LOSSLESS;
      return;
    }
    else
    {
      encapsyntax = COMPRESSED_UNKNOWN;
      return;
    }
  }

  if (!strncmp(*dicom_transfer_syntax,"1.2.840.10008.1.2.5",19))  /* RLE */
  {
    encapsyntax = COMPRESSED_RLE;
    return;
  }


  if ((*dicom_transfer_syntax)[17]!='.') {
    syntax=LITTLE|IMPLICIT;
    filesyntax=syntax; pixelsyntax=syntax;
  }else{
    switch((*dicom_transfer_syntax)[18])
    {
    case '1' :
    case '4' :
      break;
    case '2' :
      syntax=BIG|EXPLICIT;
      filesyntax=syntax; pixelsyntax=syntax;
      break;
    default :
      dicom_log(WARNING,"Unknown transfer syntax");
      dicom_log(WARNING,*dicom_transfer_syntax);
    }
  }
}

/******
 * vr *
 ******/

static void dicom_vr(void)
{
  static DICTIONARY data[]=
  {
    #include "dictionary.SQ"
  };

  dicom_log(DICOM_DEBUG,"dicom_vr()");

  element.vr=dicom_private(data,&element)->vr;
}

/****************
 * encapsulated *
 ****************/

static void dicom_encapsulated(int reset)
{
  static int encapsulated;

  dicom_log(DICOM_DEBUG,"dicom_encapsulated()");

  if (reset)
  {
    encapsulated=0;
    return;
  }

  element.encapsulated=encapsulated;

  if (encapsulated)
    if (element.group==0xFFFE)
      if (element.element==0xE0DD)
        encapsulated=0;

  if (element.length==0xFFFFFFFF)
    if (element.vr!=SQ && element.group!=0xFFFE)
      encapsulated=-1;
}

/************
 * sequence *
 ************/

static void dicom_sequence(int reset)
{
  static U32	length[0x100];
  static U8	sequence;
  static TAG    sqtag[0x100];

  dicom_log(DICOM_DEBUG,"dicom_sequence()");

  if (reset)
  {
    sequence=0;
    return;
  }

  element.sequence=sequence;

  if (sequence)
  {
    element.sqtag.group   = sqtag[sequence].group;
    element.sqtag.element = sqtag[sequence].element;

    if ((element.group == 0xFFFE) && (element.element == 0x0000))
    {
      /* skip those nasty item tags */
      dicom_log(WARNING,"Skip PHILIPS premature item bug");
      element.length=0; element.vm=0;
      fseek(stream,4,SEEK_CUR);
      return;
    }

    if (length[sequence]!=0xFFFFFFFF)
    {
      *length=ftell(stream)-position;
      if (element.length!=0xFFFFFFFF)
        if (element.group!=0xFFFE || element.element!=0xE000)
          *length+=element.length;

      if (*length>length[sequence])
      {
        dicom_log(WARNING,"Incorrect sequence length");
        sequence--;
      }
      else
        length[sequence]-=*length;

      if (!length[sequence])
        sequence--;
    }
  }

  if (element.vr==SQ) {
    if (sequence!=0xFF)
    {
      sequence++;
      length[sequence]=element.length;
      sqtag[sequence].group  = element.group;
      sqtag[sequence].element= element.element;
    }
    else
      dicom_log(WARNING,"Deep sequence hierarchy");
  }

  if (element.group==0xFFFE)
    if (element.element==0xE0DD) {
      if (!element.encapsulated) {
        if (sequence)
          sequence--;
        else
          dicom_log(WARNING,"Incorrect sequence delimiter");
      }
    }
}

/**********
 * endian *
 **********/

static void dicom_endian(void)
{
  U32	i;
  U8	*s;

  dicom_log(DICOM_DEBUG,"dicom_endian()");

  if (syntax & endian)
    return;

  switch(element.vr)
  {
  case AT :
  case OW :
  case SS :
  case US :
    s=element.value.UN;
    for (i=element.length>>1; i; i--,s+=2)
      dicom_swap(s,2);
    return;

  case SL :
  case UL :
  case FL :
    s=element.value.UN;
    for (i=element.length>>2; i; i--,s+=4)
      dicom_swap(s,4);
    return;

  case FD :
    s=element.value.UN;
    for (i=element.length>>3; i; i--,s+=8)
      dicom_swap(s,8);
    return;

  default:
    return;
  }
}

/******
 * vm *
 ******/

static int dicom_vm(void)
{
  U32	i;
  char	*c,**table,*s,*d;

  dicom_log(DICOM_DEBUG,"dicom_vm()");

  switch(element.length)
  {
  case 0 :
    element.vm=0;
    return 0;

  case 0xFFFFFFFF :
    element.vm=1;
    return 0;
  }

  switch(element.vr)
  {
    case LT :
    case OB :
    case OW :
    case SQ :
    case ST :
    case UT :
    default :
      element.vm=1;
      return 0;

    case SS :
    case US :
      element.vm=element.length>>1;
      return 0;

    case AT :
    case FL :
    case SL :
    case UL :
      element.vm=element.length>>2;
      return 0;

    case FD :
      element.vm=element.length>>3;
      return 0;

    case AE :
    case AS :
    case CS :
    case DA :
    case DS :
    case DT :
    case IS :
    case LO :
    case PN :
    case SH :
    case TM :
    case UI :
      element.vm=1;
      c=element.value.UN;
      for (i=element.length; i; i--,c++)
        if (*c=='\\')
          element.vm++;

      element.value.UN=realloc(element.value.UN,element.vm*sizeof(char*)
      +element.length+1);
      if (!element.value.UN)
      {
	dicom_log(ERROR,"Out of memory");
	dicom_close();
	return -1;
      }

      c=element.value.LT+element.vm*sizeof(char*);

      s=element.value.LT+element.length;
      d=c+element.length;
      for (i=element.length; i; i--)
        *--d=*--s;

      table=element.value.AE;
      *table++=c;

      for (i=element.length; i; i--,c++)
        if (*c=='\\')
        {
          *c=0;
          *table++=c+1;
        }

      *c=0;

      if (!(element.length&1))
        if (*--c==' ')
          *c=0;

      return 0;
  }
}

/********
 * swap *
 ********/

void dicom_swap(void *v,int n)
{
  int i;
  U8 *b,*e,tmp;

  if (syntax & endian)
    return;

  b=v;
  e=b+n-1;

  for (i=n>>1; i; i--)
  {
    tmp=*b;
    *b++=*e;
    *e--=tmp;
  }
}

/*********
 * check *
 *********/

int dicom_check(int expected)
{
  if (ferror(stream))
  {
    dicom_log(ERROR,"Error while reading file");
    dicom_close();
    return -1;
  }

  if (feof(stream))
  {
    if (!expected)
      dicom_log(ERROR,"Unexpected end of file");
    dicom_close();
    return -2;
  }

  return 0;
}

/******************
 * MDC decompress *
 ******************/

int mdc_dicom_decompress(SINGLE *s, ELEMENT *e)
{
  switch (encapsyntax) 
  {
    case COMPRESSED_RLE     :
        if (mdc_dicom_decomp_rle (stream,(U16*)e->value.OB,e->length))
          return(-1);
        break;
    case COMPRESSED_LOSSLESS:
        if (mdc_dicom_decomp_ljpg(stream,(U16*)e->value.OB,e->length
                                        ,(unsigned)s->alloc*s->samples))
          return(-2);
        break;
    case COMPRESSED_LOSSLY  :
    default:
        /* no valid decompressor */
          return(-3);
  }

  return(0);

}
