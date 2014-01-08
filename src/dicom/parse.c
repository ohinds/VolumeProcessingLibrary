/****************************
 * dicom parse by Tony Voet *
 ****************************/
/*
 * $Id: parse.c,v 1.2 2006/05/09 15:00:36 oph Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include "dicom.h"

//static DICTIONARY	*parse_input(char *);
//static void		parse_output(const DICTIONARY *);
//static void             parse_warn(const char *msg);
//
///********
// * main *
// ********/
//
//#define	LINE 8192
//
//int main(int argc,char *argv[])
//{
//  DICTIONARY	*dict;
//  char		line[LINE];
//
//  for (;;)
//  {
//    fgets(line,LINE,stdin);
//
//    if (feof(stdin))
//      break;
//
//    dict=parse_input(line);
//    if (dict)
//      parse_output(dict++);
//  }
//
//  puts("{0xFFFF,0xFFFF,ANY,  0xFFFF,0xFFFF,ANY,  UN, \"Unknown\"}");
//
//  return 0;
//}
//
///*********
// * input *
// *********/
//
//static DICTIONARY *parse_input(char *c)
//{
//  static DICTIONARY d;
//
//  if (*c=='#')
//    return 0L;
//
//  if (*c++!='(')
//  {
//    parse_warn("'(' expected"); parse_warn(c);
//    return 0L;
//  }
//
//  d.group=strtol(c,&c,16);
//  d.group_last=d.group;
//  d.group_match=ANY;
//
//  if (*c=='-')
//  {
//    switch(*++c)
//    {
//    default :
//      d.group_match=EVEN;
//      break;
//    case 'u' :
//      c+=2;
//      break;
//    case 'o' :
//      d.group_match=ODD;
//      c+=2;
//    }
//    d.group_last=strtol(c,&c,16);
//  }
//
//  if (*c++!=',')
//  {
//    parse_warn("',' expected"); parse_warn(c);
//    return 0L;
//  }
//
//  d.element=strtol(c,&c,16);
//  d.element_last=d.element;
//  d.element_match=ANY;
//
//  if (*c=='-')
//  {
//    switch(*++c)
//    {
//    default :
//      d.element_match=EVEN;
//      break;
//    case 'u' :
//      c+=2;
//      break;
//    case 'o' :
//      d.element_match=ODD;
//      c+=2;
//    }
//    d.element_last=strtol(c,&c,16);
//  }
//
//  if (*c++!=')')
//  {
//    parse_warn("')' expected"); parse_warn(c);
//    return 0L;
//  }
//
//  if (*c++!='\t')
//  {
//    parse_warn("'\\t' expected"); parse_warn(c);
//    return 0L;
//  }
//
//  d.vr=*c++<<8;
//  d.vr|=*c++;
//
//  switch(d.vr)
//  {
//  case AE :
//  case AS :
//  case AT :
//  case CS :
//  case DA :
//  case DS :
//  case DT :
//  case FL :
//  case FD :
//  case IS :
//  case LO :
//  case LT :
//  case OB :
//  case OW :
//  case PN :
//  case SH :
//  case SL :
//  case SQ :
//  case SS :
//  case ST :
//  case TM :
//  case UI :
//  case UL :
//  case US :
//  case UN :
///* special tag (choice) */
//  case ox :
//    break;
//
//  default :
//    d.vr=UN;
//  }
//
//  if (*c++!='\t')
//  {
//    parse_warn("'\\t' expected"); parse_warn(c);
//    return 0L;
//  }
//
//  d.description=c;
//
//  for (; *c!='\t'; c++);
//
//  *c=0;
//
//  return &d;
//}
//
///**********
// * output *
// **********/
//
//static void parse_output(const DICTIONARY *d)
//{
//  static char *match[]=
//  {
//    "EVEN,",
//    "ODD, ",
//    "ANY, "
//  };
//
//  if (!d)
//    return;
//
//  printf
//  (
//    "{0x%.4X,0x%.4X,%s 0x%.4X,0x%.4X,%s %c%c, \"%s\"},\n",
//
//    d->group,
//    d->group_last,
//    match[d->group_match],
//
//    d->element,
//    d->element_last,
//    match[d->element_match],
//
//    d->vr>>8,
//    d->vr&0xFF,
//    
//    d->description
//  );
//}
//
//void parse_warn(const char *message)
//{
//  fprintf(stderr,"parse: WARNING: %s\n",message);
//}
