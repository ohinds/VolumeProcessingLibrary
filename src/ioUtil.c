/*****************************************************************************
 * ioUtil.c is the source file for input/output utility functions for libvp
 * Oliver Hinds <oph@bu.edu> 2005-06-02
 *
 *
 *
 *****************************************************************************/

#include"ioUtil.h"

/** general file io functions **/

/**
 * open a file and return a vpFile descriptor, or NULL on failure
 */
vpFile *vpOpen(const char *filename, const char *mode) {
//  vpFile *vpf;
//  FILE *fp;
//
//  /* validate */
//  if(filename == NULL || mode == NULL) {
//    return NULL;
//  }
//
//  /* try to open */
//  fp = fopen(filename,mode);
//  if(fp == NULL) {
//    return NULL;
//  }
//
//  /* allocate the vpFile and assign fields */
//  vpf = (vpFile*) malloc(sizeof(vpFile));
//  vpf->fp = fp;
//  vpf->zfp = gzdopen((int)fp,mode);
//
//  vpf->open = TRUE;
//
//  return vpf;
  return NULL;
}

/**
 * closes a vpFile descriptor
 */
void vpClose(vpFile *vfp) {
//  if(vfp != NULL && vfp->open) {
//    fclose(vfp->fp);
//    gzclose(vfp->zfp);
//    vfp->open = FALSE;
//  }
}

/**
 * converts an integer to a string
 * original by Manuel Novoa III.
 */
char *itoa(int i) {
  /* 10 digits + 1 sign + 1 trlibvping nul */
  static char buf[12];

  char *pos = buf + sizeof(buf) - 1;
  unsigned int u;
  int negative = 0;

  if (i < 0) {
    negative = 1;
    u = ((unsigned int)(-(1+i))) + 1;
  }
  else {
    u = i;
  }

  *pos = 0;

  do {
    *--pos = '0' + (u % 10);
    u /= 10;
  } while (u);

  if (negative) {
    *--pos = '-';
  }

  return pos;
}

/** file reading utils **/

/**
 * skip all white space
 */
void skipWhite(FILE* f){
  char c;
  while(isspace(c = getc(f)));
  ungetc(c,f);
}

/**
 * if next non whitespace char is symbol, treat rest of line as a comment
 */
void skipComments(FILE* f, char symbol){
  char c;

  skipWhite(f);

  while(symbol == (c = getc(f))) { /* found a comment, read to end of line */
    while('\n' != (c = getc(f)));
  }
  ungetc(c,f);

  skipWhite(f);
}

/**
 * reads all characters until an end line in reached
 */
void readLine(FILE *fp, char *buf) {
  char c = getc(fp);
  int i;

  for(i = 0; !feof(fp) && c != '\n'; i++) {
    buf[i] = c;
    c = getc(fp);
  }

  /* terminate */
  buf[i] = '\0';
}

/**
 * reads n strings from a file stream into an array of strings,
 * returns 1 for success, 0 for flibvpure
 */
char **readStrings(FILE *fp, int n) {
  int i;

  /* allocate the array of strings */
  char **buf = (char**) malloc(n*sizeof(char*));

  /* read each string */
  for(i = 0; i < n; i++) {
    /* check for end of file */
    if(feof(fp)) {
      return NULL;
    }

    /* allocate and read a string */
    buf[i] = (char*) malloc(VP_MAX_STR_LEN*sizeof(char));
    fscanf(fp,"%s",buf[i]);
  }

  return buf;
}

/**
 * reads voxels for a volume from a file stream
 */
int readVolumeVoxels(FILE* fp, volume *vol) {
  unsigned char *buf1;
  unsigned int *buf4;
  float *buf4f;
  int numVox;

  /* validate */
  if(vol == NULL || feof(fp)) {
    fprintf(stderr,"couldn't read voxels from stream.\n");
    return VP_FAILURE;
  }

  numVox = vol->size[FRAME]*vol->size[ROW]*vol->size[COL]*vol->size[SLICE]*vol->size[ECHO]*vol->size[CHANNEL];
  /* allocate space for the voxels and assign an actual bpp based on known
     types */
  switch(vol->type) {
  case UCHAR:
    buf1 = (unsigned char*) malloc(numVox*sizeof(unsigned char));

    if(VP_DEBUG) {
      fprintf(stderr,"reading %d unsigned char voxels...", numVox);
    }

    /* forward to the start of the frame and read it */
    fread(buf1, sizeof(unsigned char), numVox, fp);

    if(VP_DEBUG) {
      fprintf(stderr,"done.\nconverting to 16 bit...");
    }

    bitDepthConvert8to16(buf1,vol->voxels,numVox);

    if(VP_DEBUG) {
      fprintf(stderr,"done.\n");
    }

    free(buf1);

    vol->actualBPP = 8*sizeof(unsigned char);
    break;
  case SHORT:
    if(VP_DEBUG) {
      fprintf(stderr,"reading %d unsigned short voxels...", numVox);
    }

    /* forward to the start of the frame and read it */
    readBigEndianShort16(fp, numVox, vol->voxels);

    if(VP_DEBUG) {
      fprintf(stderr,"done.\n");
    }

    vol->actualBPP = 8*sizeof(unsigned short);
    break;
  case INT:
  case LONG:
    buf4 = (unsigned int*) malloc(numVox*sizeof(unsigned int));

    if(VP_DEBUG) {
      fprintf(stderr,"reading %d unsigned int...", numVox);
    }

    /* forward to the start of the frame and read it */
    readBigEndianInt32(fp, numVox, buf4);

    if(VP_DEBUG) {
      fprintf(stderr,"done.\nconverting to 16 bit...");
    }

    bitDepthConvert32to16(buf4,vol->voxels,numVox);

    if(VP_DEBUG) {
      fprintf(stderr,"done.\n");
    }

    free(buf4);

    vol->actualBPP = 8*sizeof(unsigned int);
    break;
  case FLOAT:
    buf4f = (float*) malloc(numVox*sizeof(float));

    if(VP_DEBUG) {
      fprintf(stderr,"reading %d float...", numVox);
    }

    /* forward to the start of the frame and read it */
    readBigEndianFloat32(fp, numVox, buf4f);

    if(VP_DEBUG) {
      fprintf(stderr,"done.\nconverting to 16 bit...");
    }

    bitDepthConvert32fto16(buf4f,vol->voxels,numVox);

    if(VP_DEBUG) {
      fprintf(stderr,"done.\n");
    }

    free(buf4f);

    vol->actualBPP = 8*sizeof(float);
    break;
  default:
    break;
  }
  vol->type = SHORT;

  /* check the allocation */
  if(vol->voxels == NULL) {
    fprintf(stderr,"failed in read of voxels from stream.\n");
    return VP_FAILURE;
  }

  return VP_SUCCESS;
}

/**
 * gets the number of bytes left in the file stream
 */
long getNumBytesLeftInFile(FILE *fp) {
  long curpos, endpos;

  /* validate */
  if(fp == NULL || feof(fp)) {
    return 0;
  }

  curpos = ftell(fp);
  fseek(fp, 0, SEEK_END);
  endpos = ftell(fp);
  fseek(fp, curpos-endpos, SEEK_END);

  return endpos-curpos;
}

/**
 * reads a certain number of elements into a short buffer, converting from
 * other types, making the best guess at the file's datatype\
 * DONT USE THIS IF THERE IS EXTRA DATA AT THE END OF A FILE
 */
int readDataIntoShortBuffer(FILE *fp, int numElements, unsigned short *buf) {
  unsigned char *charBuf;
  float *floatBuf;
  long bytesLeft = 0;

  /* validate */
  if(fp == NULL || feof(fp) || buf == NULL) {
    return VP_FAILURE;
  }

  /* check the number of bytes left in the file */
  bytesLeft = getNumBytesLeftInFile(fp);
  
  if(bytesLeft == 0 || bytesLeft < numElements) {
    if(VP_DEBUG) {
      printf("ioUtil.c: not enough bytes left to read\n");
    }

    return VP_FAILURE;
  }

  /* compare the number of bytes left to the number we'd need to read in the
     data of each type */
  if(numElements <= bytesLeft 
     && sizeof(unsigned short)*numElements > bytesLeft) { /* assume char */
    if(VP_DEBUG) {
      printf("ioUtil.c: assuming the type is bytes\n");
    }

    charBuf = (unsigned char*) malloc(numElements*sizeof(unsigned char));
    fread(charBuf, sizeof(unsigned char), numElements, fp);
    bitDepthConvert8to16(charBuf,buf,numElements);
    free(charBuf);
  }
  else if(numElements <= sizeof(unsigned short)*bytesLeft 
	  && sizeof(float)*numElements > bytesLeft) { /* assume short */
    if(VP_DEBUG) {
      printf("ioUtil.c: assuming the type is shorts\n");
    }

    readBigEndianShort16(fp, numElements, buf);    
  }
  else if(numElements <= sizeof(float)*bytesLeft) { /* assume float */
    if(VP_DEBUG) {
      printf("ioUtil.c: assuming the type is floats\n");
    }

    floatBuf = (float*) malloc(numElements*sizeof(float));
    readBigEndianFloat32(fp,numElements,floatBuf);
    bitDepthConvert32fto16(floatBuf,buf,numElements);
    free(floatBuf);
  }
  else {
    if(VP_DEBUG) {
      printf("ioUtil.c: couldn't guess byte type with %d elements and %ld bytes left.\n", numElements, bytesLeft);
    }

    return VP_FAILURE;
  }

  return VP_SUCCESS;
}


/**
 * reads n big endian byte ordered shorts
 */
int readBigEndianShort16(FILE *fp, int n, unsigned short *dest) {
  int i;

  /* validate input */
  if(fp == NULL || feof(fp) || dest == NULL) return VP_FAILURE;

  /* read the block of shorts */
  fread(dest, sizeof(unsigned short), n, fp);

  /* swap the bytes if little endian */
  if(ENDIAN == BIG) {
    return VP_SUCCESS;
  }

  for(i = 0; i < n; i++) {
    dest[i] = swapByteOrder16(dest[i]);
  }

  return VP_SUCCESS;
}

/**
 * writes n big endian byte ordered shorts
 */
int writeBigEndianShort16(FILE *fp, int n, unsigned short *src) {
  int i;
  
  /* validate input */
  if(fp == NULL || src == NULL || n == 0) return VP_FAILURE;

  /* swap bytes if little endian */
  if(ENDIAN == LITTLE) {
    for(i = 0; i < n; i++) {
      src[i] = swapByteOrder16(src[i]);
    }
  }

  /* write the bytes */
  fwrite(src, sizeof(unsigned short), n, fp);

  /* swap back */
  if(ENDIAN == LITTLE) {
    for(i = 0; i < n; i++) {
      src[i] = swapByteOrder16(src[i]);
    }
  }

  return VP_SUCCESS;
}

/**
 * reads n big endian byte ordered 3 byte ints
 */
int readBigEndianInt24(FILE *fp, int n, unsigned int *dest) {
  int i;
  int b1, b2, b3;

  /* validate input */
  if(fp == NULL || feof(fp) || dest == NULL) return VP_FAILURE;

  /* read each int */
  for(i = 0; i < n; i++) {
    fread(&b1, sizeof(char), 1, fp);
    fread(&b2, sizeof(char), 1, fp);
    fread(&b3, sizeof(char), 1, fp);
    dest[i] = 0x00ffffff & ((0x00ff0000 & (b1<<16))
			  | (0x0000ff00 & (b2<<8))
			  | (0x000000ff & b3));
  }

  return VP_SUCCESS;
}

/**
 * writes n big endian byte ordered 3 byte ints
 */
int writeBigEndianInt24(FILE *fp, int n, unsigned int *src) {
  int i;
  char b1, b2, b3;
  
  /* validate input */
  if(fp == NULL || src == NULL || n == 0) return VP_FAILURE;

  for(i = 0; i < n; i++) {
    b1 = 0x000000ff & (src[i]>>16);
    b2 = 0x000000ff & (src[i]>>8);
    b3 = 0x000000ff & src[i];

    fwrite(&b1, sizeof(char), 1, fp);
    fwrite(&b2, sizeof(char), 1, fp);
    fwrite(&b3, sizeof(char), 1, fp);
  }

  return VP_SUCCESS;
}

/**
 * reads n big endian byte ordered ints
 */
int readBigEndianInt32(FILE *fp, int n, unsigned int *dest) {
  int i;

  /* validate input */
  if(fp == NULL || feof(fp) || dest == NULL) return VP_FAILURE;

  /* read the block of shorts */
  fread(dest, sizeof(unsigned int), n, fp);

  /* swap the bytes if little endian */
  if(ENDIAN == BIG) {
    return VP_SUCCESS;
  }

  for(i = 0; i < n; i++) {
    dest[i] = swapByteOrder32(dest[i]);
  }

  return VP_SUCCESS;
}

/**
 * writes n big endian byte ordered ints
 */
int writeBigEndianInt32(FILE *fp, int n, unsigned int *src) {
  int i;
  
  /* validate input */
  if(fp == NULL || src == NULL || n == 0) return VP_FAILURE;

  /* swap bytes if little endian */
  if(ENDIAN == LITTLE) {
    for(i = 0; i < n; i++) {
      src[i] = swapByteOrder32(src[i]);
    }
  }

  /* write the bytes */
  fwrite(src, sizeof(unsigned int), n, fp);

  /* swap back */
  if(ENDIAN == LITTLE) {
    for(i = 0; i < n; i++) {
      src[i] = swapByteOrder32(src[i]);
    }
  }

  return VP_SUCCESS;
}

/**
 * reads n big endian byte ordered floats
 */
int readBigEndianFloat32(FILE *fp, int n, float *dest) {
  int i;

  /* validate input */
  if(fp == NULL || feof(fp) || dest == NULL) return VP_FAILURE;

  /* read the block of floats */
  fread(dest, sizeof(float), n, fp);

  /* swap the bytes if little endian */
  if(ENDIAN == BIG) {
    return VP_SUCCESS;
  }

  for(i = 0; i < n; i++) {
    dest[i] = swapByteOrder32f(dest[i]);
  }

  return VP_SUCCESS;
}

/**
 * writes n big endian byte ordered floats
 */
int writeBigEndianFloat32(FILE *fp, int n, float *src) {
  int i;
  
  /* validate input */
  if(fp == NULL || src == NULL || n == 0) return VP_FAILURE;

  /* swap bytes if little endian */
  if(ENDIAN == LITTLE) {
    for(i = 0; i < n; i++) {
      src[i] = swapByteOrder32f(src[i]);
    }
  }

  /* write the bytes */
  fwrite(src, sizeof(float), n, fp);

  /* swap back */
  if(ENDIAN == LITTLE) {
    for(i = 0; i < n; i++) {
      src[i] = swapByteOrder32f(src[i]);
    }
  }

  return VP_SUCCESS;
}

/**
 * reads n big endian byte ordered doubles
 */
int readBigEndianDouble64(FILE *fp, int n, double *dest) {
  int i;

  /* validate input */
  if(fp == NULL || feof(fp) || dest == NULL) return VP_FAILURE;

  /* read the block of bytes */
  fread(dest, sizeof(double), n, fp);

  /* swap the bytes if little endian */
  if(ENDIAN == BIG) {
    return VP_SUCCESS;
  }

  for(i = 0; i < n; i++) {
    dest[i] = swapByteOrder64f(dest[i]); 
  }

  return VP_SUCCESS;
}

/**
 * writes n big endian byte ordered doubles
 */
int writesBigEndianDouble64(FILE *fp, int n, double *src) {
  int i;
  
  /* validate input */
  if(fp == NULL || src == NULL || n == 0) return VP_FAILURE;

  /* swap bytes if little endian */
  if(ENDIAN == LITTLE) {
    for(i = 0; i < n; i++) {
      src[i] = swapByteOrder64f(src[i]);
    }
  }

  /* write the bytes */
  fwrite(src, sizeof(double), n, fp);

  /* swap back */
  if(ENDIAN == LITTLE) {
    for(i = 0; i < n; i++) {
      src[i] = swapByteOrder64f(src[i]);
    }
  }

  return VP_SUCCESS;
}

/**
 * swap the byte order of a 32 bit int or long
 */
unsigned short swapByteOrder16(unsigned short n) {
  return (((n>>8)) | (n<<8));
}

/**
 * swap the byte order of a 32 bit int or long
 */
unsigned int swapByteOrder32(unsigned int n) {
   return (((n&0x000000FF)<<24)+((n&0x0000FF00)<<8)+((n&0x00FF0000)>>8)
	   +((n&0xFF000000)>>24));
}

/**
 * swap the byte order of a 32 bit float
 */
float swapByteOrder32f(float f) {
  floatUnion fu;
  unsigned short s;

  /* first swap bytes in each word */
  fu.f = f;
  fu.s[0] = swapByteOrder16(fu.s[0]);
  fu.s[1] = swapByteOrder16(fu.s[1]);
  
  /* now swap words */
  s = fu.s[0];
  fu.s[0] = fu.s[1];
  fu.s[1] = s;
  
  return fu.f;
}

/**
 * swap the byte order of a 64 bit number as two 32 bit numbers
 */
double swapByteOrder64f(double d) {
  doubleUnion du;
  unsigned char tmp;
  int i;

  du.d = d;

  for(i = 0; i < 4; i++) {
    tmp = du.bytes[i];
    du.bytes[i] = du.bytes[7-i];
    du.bytes[7-i] = tmp;
  }

  return du.d;
}

/**
 * convert a byte array into one float number
 */
float getFloat32FromBytes(unsigned char byteArray[4]) {
  floatUnion fu;
  int i;

  for(i = 0; i < 3; i++) {
    fu.bytes[i] = byteArray[i];
  }

  return fu.f;
}

/**
 * convert a byte array into one double number
 */
double getDouble64FromBytes(unsigned char byteArray[8]) {
  doubleUnion du;
  int i;

  for(i = 0; i < 7; i++) {
    du.bytes[i] = byteArray[i];
  }

  return du.d;
}

/**
 * looks for a valid image or volume type name in a string,
 * returns both the format and type (volume or image)
 */
int getTypeAndFormatFromString(char *str, enum IMAGEFORMAT *format, 
			       enum INPUTTYPE *type) {
  /* validate */
  if(str == NULL || str[0] == '\0') {
    *format = INVALID_FORMAT;
    *type = INVALID_INPUT_TYPE;
    return VP_FAILURE;
  }

  /* look for a valid type */  
  if(!strcmp(str,"jpeg") || !strcmp(str,"jpg") ||
     !strcmp(str,"JPEG") || !strcmp(str,"JPG")) {
    *type = VP_IMAGE;
    *format = JPEG;
  }
  else if(!strcmp(str,"pnm") || !strcmp(str,"PNM") ||
	  !strcmp(str,"ppm") || !strcmp(str,"PPM")) {
    *type = VP_IMAGE;
    *format = PNM;
  }
  else if(!strcmp(str,"dicom") || !strcmp(str,"DICOM") ||
	  !strcmp(str,"dcm") || !strcmp(str,"DCM")) {
    *type = VP_IMAGE;
    *format = DICOM;
  }
  else if(!strcmp(str,"dat") || !strcmp(str,"DAT")) {
    *type = VP_IMAGE;
    *format = DAT;
  }  
  else if(!strcmp(str,"mgh") || !strcmp(str,"MGH")) {
    *type = VP_VOLUME;
    *format = MGH;
  }
  else if(!strcmp(str,"nii") || !strcmp(str,"NII")) {
    *type = VP_VOLUME;
    *format = NIFTI;
  }
  else if(!strcmp(str,"raw") || !strcmp(str,"RAW")) {
    *type = VP_VOLUME;
    *format = RAW;
  }  
  else if(!strcmp(str,"rawiv") || !strcmp(str,"RAWIV")) {
    *type = VP_VOLUME;
    *format = RAWIV;
  }
  else {
    *format = INVALID_FORMAT;
    *type = INVALID_INPUT_TYPE;
    return VP_FAILURE;
  }

  return VP_SUCCESS;
}

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/ioUtil.c,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/

