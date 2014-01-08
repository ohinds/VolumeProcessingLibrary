/*************************
 * libdicom by Tony Voet *
 *************************/
/* 
 * $Id: log.c,v 1.1 2005/07/28 19:25:50 oph Exp $
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "dicom.h"

CONDITION dicom_log_level=NOTICE;

/* eNlf: BEGIN -- change for compilation error on Red Hat 6.0 */
/* static FILE     *stream=stderr;                            */
/* The above statement fails: initializer not constant        */
/* eNlf: END   -- change for compilation error on Red Hat 6.0 */
static FILE	*stream=NULL;
static char	*program=NULL;

/************
 * log name *
 ************/

void dicom_log_name(char *name)
{
  program=strrchr(name,'/');

  if (program)
    program++;
  else
    program=name;
}

/************
 * log open *
 ************/

int dicom_log_open(const char *file)
{
  if (!file)
  {
    dicom_log(ERROR,"No file given");
    return -1;
  }

  stream=fopen(file,"a");

  if (!stream)
  {
    stream=stderr;
    dicom_log(ERROR,"Unable to open log file");
    return -1;
  }

  return 0;
}

/*******
 * log *
 *******/

void dicom_log(CONDITION condition,const char *message)
{
  time_t	t;
  char		tmp[32];

  static char *explination[]=
  {
    "emergency",
    "alert",
    "critical",
    "error",
    "warning",
    "notice",
    "info",
    "debug"
  };

  if (condition>dicom_log_level)
    return;

  time(&t);
  strftime(tmp,32,"%b %d %H:%M:%S",localtime(&t));

/* eNlf: BEGIN  -- change for compilation error on Red Hat 6.0 */
  if (stream == NULL) {
    fprintf(stderr,"%s %s[%u]: %s: %s\n",
      tmp,
      program ? program : "log",
      (unsigned int) getpid(),
      explination[condition],
      message);
  }else{
    fprintf(stream,"%s %s[%u]: %s: %s\n",
      tmp,
      program ? program : "log",
      (unsigned int) getpid(),
      explination[condition],
      message);
  }
/* eNlf: END   -- change for compilation error on Red Hat 6.0 */
}

/*************
 * log close *
 *************/

int dicom_log_close(void)
{
  if (stream==stderr)
  {
    dicom_log(NOTICE,"Attempt to close stderr");
    return -1;
  }

  if (fclose(stream))
  {
    stream=stderr;
    dicom_log(WARNING,"Unable to close log");
    return -2;
  }

  stream=stderr;

  return 0;
}
