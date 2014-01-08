/*****************************************************************************
 * volumeIO.c is the source file for the volume reading/writing functions
 * for libvp
 * Oliver Hinds <oph@bu.edu> 2005-04-12
 *
 *
 *
 *****************************************************************************/

#define VOLUME_IO_VERSION_C "$Id: volumeIO.c,v 1.15 2007/08/04 20:29:40 oph Exp $"

#include"volumeIO.h"

/**
 * loads a volume based on a passed type
 */
volume *loadVolume(char *filename, int format) {
  switch(format) {
  case MGH:
    return loadMGHVolume(filename);
    break;
  case RAW:
    return loadRAWVolume(filename);
    break;
  case RAWIV:
    return loadRAWIVVolume(filename);
    break;
  case NIFTI:
    return loadNiftiVolume(filename);
    break;
  default:
    fprintf(stderr,"loadVolume(): unsupported format\n");
    break;
  }

  return NULL;
}

/**
 * load a volume
 * returns a volume struct
 */
volume *loadVolumeFromImages(char **filenames, int numImages, int format) {
  int imInd;
  volume *vol;
  image *img = NULL;

  /* validate */
  if(filenames == NULL || numImages < 1) {
    return NULL;
  }

  /* load tyhe first slice to get width and height info */
  switch(format) {
  case DICOM:
    img = readDICOM(filenames[0]);
    break;
  case JPEG:
    img = readJPEG(filenames[0]);
    break;
  case TIFF:
    img = readTIFF(filenames[0]);
    break;
  case PNM:
    img = readPNM(filenames[0]);
    break;
  default:
    fprintf(stderr,"loadVolumeFromImages error: unsupported image format\n");
    break;
  }

  /* validate */
  if(img == NULL) {
    fprintf(stderr,"loadVolumeFromImages error: couldn't load first image\n");
    return NULL;
  }

  /* allocate volume based on image params */
  vol = createVolume(1, img->width, img->height, numImages, 1, img->numChannels);

  /* validate volume allocation */
  if(vol == NULL) {
    fprintf(stderr,"loadVolumeFromImages error: couldn't allocate volume\n");
    freeImage(img);
    return NULL;
  }

  /* copy the first slice */
  copyPixels(vol,img,0);
  freeImage(img);

  /* read each image */
  for(imInd = 1; imInd < numImages; imInd++) {

    /* load a slice by calling the appropriate image load function */
    switch(format) {
    case DICOM:
      img = readDICOM(filenames[imInd]);
      break;
    case JPEG:
      img = readJPEG(filenames[imInd]);
      break;
    case TIFF:
      img = readTIFF(filenames[imInd]);
      break;
    case PNM:
      img = readPNM(filenames[imInd]);
      break;
    default:
      fprintf(stderr,"error: unsupported image format\n");
      break;
    }
    if(img == NULL) continue;

    if(img->height != vol->size[ROW] || img->width != vol->size[COL]) {
      fprintf(stderr,"error: all images for a volume must be of the same dimensions.\nvol dim=<%d,%d>, img dim=<%d,%d>\n",
	      vol->size[COL],vol->size[ROW],img->width,img->height);
      return NULL;
    }

    copyPixels(vol,img,imInd);
    freeImage(img);
  }

  return vol;
}

/**
 * reads the header of an mgh file
 */
int readMGHHeader(FILE* fp, volume **headerVol) {
  volume *vol;
  unsigned short rasGoodFlag;
  unsigned int v,rows,cols,slices,frames,numVox,dof,type;
  float Pxyz_c[3], Pxyz_0[3], Pcrs_c[3], Ds[3][3],
    tmpProd3by3[3][3], tmpProd3by1[3], Mdc[3][3];

  int UNUSED_SPACE_SIZE= 256;
  int USED_SPACE_SIZE = (3*4+4*3*4);  /* space for raster transform */
  int unusedSpaceSize = UNUSED_SPACE_SIZE-2 ;

  /* validate the file */
  if(fp == NULL) {
    return VP_FAILURE;
  }

  /* read the header */
  readBigEndianInt32(fp,1,&v);
  readBigEndianInt32(fp,1,&rows);
  readBigEndianInt32(fp,1,&cols);
  readBigEndianInt32(fp,1,&slices);
  readBigEndianInt32(fp,1,&frames);
  readBigEndianInt32(fp,1,&type);
  readBigEndianInt32(fp,1,&dof);

  /* allocate the volume */
  vol = createEmptyVolume(frames,rows,cols,slices,1,1);
  /* validate allocation */
  if(vol == NULL) {
    fprintf(stderr,"error: could not allocate volume to read the mgh header\n");
    return VP_FAILURE;
  }

  if(VP_DEBUG) {
    fprintf(stderr,
	    "v=%d\nrows=%d\ncols=%d\nslices=%d\nframes=%d\ntype=%d\ndof=%d\n",
	    v,vol->size[ROW],vol->size[COL],vol->size[SLICE],vol->size[FRAME],
	    type,dof);
  }

  vol->type = type;

  /* read the existence of a coordinate header */
  readBigEndianShort16(fp,1,&rasGoodFlag);

  if(VP_DEBUG) {
    fprintf(stderr,"rasGoodFlag=%d\n",rasGoodFlag);
  }

  /* assign the total number of voxels */
  numVox = vol->size[FRAME]*vol->size[ROW]*vol->size[COL]*vol->size[SLICE];

  if(rasGoodFlag) { /* read the rest of the header */
    /* read diagonal entries of D and zero the rest */
    readBigEndianFloat32(fp,1,&Ds[0][0]);
    readBigEndianFloat32(fp,1,&Ds[1][1]);
    readBigEndianFloat32(fp,1,&Ds[2][2]);
    Ds[0][1] = Ds[0][2] = Ds[1][0] = Ds[1][2] = Ds[2][0] = Ds[2][1] = 0.0;

    /* read the 3x3 mdc matrix */
    readBigEndianFloat32(fp,1,&Mdc[0][0]);
    readBigEndianFloat32(fp,1,&Mdc[1][0]);
    readBigEndianFloat32(fp,1,&Mdc[2][0]);
    readBigEndianFloat32(fp,1,&Mdc[0][1]);
    readBigEndianFloat32(fp,1,&Mdc[1][1]);
    readBigEndianFloat32(fp,1,&Mdc[2][1]);
    readBigEndianFloat32(fp,1,&Mdc[0][2]);
    readBigEndianFloat32(fp,1,&Mdc[1][2]);
    readBigEndianFloat32(fp,1,&Mdc[2][2]);

    /* read something */
    readBigEndianFloat32(fp,3,Pxyz_c);

    /* generate crs vector */
    Pcrs_c[0] = vol->size[ROW]/2.0;
    Pcrs_c[1] = vol->size[COL]/2.0;
    Pcrs_c[2] = vol->size[SLICE]/2.0;

    /* now calculate the M matrix */
    matrixMult3by3(Mdc,Ds,tmpProd3by3);
    matrixMult3by1(tmpProd3by3,Pcrs_c,tmpProd3by1);

    /* perform a subtraction */
    Pxyz_0[0] = Pxyz_c[0] - tmpProd3by1[0];
    Pxyz_0[1] = Pxyz_c[1] - tmpProd3by1[1];
    Pxyz_0[2] = Pxyz_c[2] - tmpProd3by1[2];

    /* assign the values of the world coord transform matrix */
    vol->vox2wrld[0][0] = tmpProd3by3[0][0];
    vol->vox2wrld[0][1] = tmpProd3by3[0][1];
    vol->vox2wrld[0][2] = tmpProd3by3[0][2];

    vol->vox2wrld[1][0] = tmpProd3by3[1][0];
    vol->vox2wrld[1][1] = tmpProd3by3[1][1];
    vol->vox2wrld[1][2] = tmpProd3by3[1][2];

    vol->vox2wrld[2][0] = tmpProd3by3[2][0];
    vol->vox2wrld[2][1] = tmpProd3by3[2][1];
    vol->vox2wrld[2][2] = tmpProd3by3[2][2];

    vol->vox2wrld[0][3] = Pxyz_0[0];
    vol->vox2wrld[1][3] = Pxyz_0[1];
    vol->vox2wrld[2][3] = Pxyz_0[2];

    vol->vox2wrld[3][0] = 0.0;
    vol->vox2wrld[3][1] = 0.0;
    vol->vox2wrld[3][2] = 0.0;
    vol->vox2wrld[3][3] = 1.0;

    unusedSpaceSize -= USED_SPACE_SIZE;

    if(VP_DEBUG) {
      fprintf(stderr,"vox2wrld=\n%02.3f %02.3f %02.3f %02.3f\n%02.3f %02.3f %02.3f %02.3f \n%02.3f %02.3f %02.3f %02.3f \n%02.3f %02.3f %02.3f %02.3f\n",
	      vol->vox2wrld[0][0], vol->vox2wrld[0][1],
	      vol->vox2wrld[0][2], vol->vox2wrld[0][3],
	      vol->vox2wrld[1][0], vol->vox2wrld[1][1],
	      vol->vox2wrld[1][2], vol->vox2wrld[1][3],
	      vol->vox2wrld[2][0], vol->vox2wrld[2][1],
	      vol->vox2wrld[2][2], vol->vox2wrld[2][3],
	      vol->vox2wrld[3][0], vol->vox2wrld[3][1],
	      vol->vox2wrld[3][2], vol->vox2wrld[3][3]
	      );
    }
  }

  /* forward to the start of the binary data */
  fseek(fp, unusedSpaceSize, SEEK_CUR);

  *headerVol = vol;

  return VP_SUCCESS;
}

/**
 * loads one volume from one MGH (up to 6d) volume file.
 * implementation follows the matlab script load_mgh.m distributed with
 * freesurfer
 *
 * returns a volume struct
 */
volume *loadMGHVolume(char *filename) {
  FILE *fp;
  volume *vol;
  unsigned int numVox;

  if(VP_DEBUG) {
    fprintf(stderr,"begin reading mgh file %s\n", filename);
  }

  /* open the file */
  fp = fopen(filename,"r");
  if(fp == NULL) {
    fprintf(stderr,"error: could not open file %s for reading an MGH volume\n",
	    filename);
    return NULL;
  }

  /* read the header */
  if(VP_FAILURE == readMGHHeader(fp,&vol)) {
    fprintf(stderr,"error: could not read the header for file %s\n", filename);
    return NULL;
  }

  /* allocate the voxels */
  numVox = vol->size[FRAME]*vol->size[ROW]*vol->size[COL]*vol->size[SLICE];
  /* allocate the voxels */
  vol->voxels = (unsigned short*) malloc(numVox*sizeof(unsigned short));

  /* validate allocation */
  if(vol->voxels == NULL) {
    vol->size[FRAME] = vol->size[ROW] = vol->size[COL] = vol->size[SLICE] = 0;
    freeVolume(vol);
    return NULL;
  }

  /* read the voxels */
  readVolumeVoxels(fp,vol);

  /* read mr parms */
  if(!feof(fp)) {
    vol->mriParms = (struct mriParameters*) malloc(sizeof(mriParameters));

    readBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->tr);
    readBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->flipangle);
    readBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->te);
    readBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->ti);
    readBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->fov);
  }

  fclose(fp);

  if(VP_DEBUG) {
    fprintf(stderr,"done reading mgh file %s\n", filename);
  }

  return vol;
}

/**
 * writes a volume struct to an MGH volume file
 * returns SUCCESS OR FAILURE
 */
int writeMGHVolume(volume *vol, char *filename) {
  FILE *fp;
  unsigned short ons = 1;
  int i, j;
  unsigned int one = 1, numVox, mriShort = SHORT;
  float MdcD[3][3], Mdc[9], delta[3], Pxyz_c[3], Pcrs_c[3];

  const int UNUSED_SPACE_SIZE= 256;
  char unusedArr[UNUSED_SPACE_SIZE];
  int USED_SPACE_SIZE = (3*4+4*3*4);  /* space for raster transform */
  int unusedSpaceSize = UNUSED_SPACE_SIZE-2 ;

  /* validate the input */
  if(vol == NULL || vol->voxels == NULL || filename == NULL || filename[0] == '\0') {
    return VP_FAILURE;
  }

  /* open the file */
  fp = fopen(filename,"w");
  if(fp == NULL) {
    fprintf(stderr,"error: could not open file %s for writing an MGH volume\n",
	    filename);
    return VP_FAILURE;
  }

  /* write the header */
  writeBigEndianInt32(fp,1,&one);
  writeBigEndianInt32(fp,3,vol->size);
  writeBigEndianInt32(fp,1,&one);
  writeBigEndianInt32(fp,1,&mriShort);
  writeBigEndianInt32(fp,1,&one);

  /* write the existence of a coordinate header */
  writeBigEndianShort16(fp,1,&ons);

  /* assign the total number of voxels */
  numVox = vol->size[ROW]*vol->size[COL]*vol->size[SLICE];

  /* create the coord system stuff for the header */

  /* get the first 3 rows and cols of M */
  memcpy(MdcD[0],vol->vox2wrld[0],4*3);
  memcpy(MdcD[1],vol->vox2wrld[1],4*3);
  memcpy(MdcD[2],vol->vox2wrld[2],4*3);

  delta[0] = sqrtf(MdcD[0][0]*MdcD[0][0]
		 + MdcD[0][1]*MdcD[0][1]
		 + MdcD[0][2]*MdcD[0][2]);

  delta[1] = sqrtf(MdcD[1][0]*MdcD[1][0]
		 + MdcD[1][1]*MdcD[1][1]
		 + MdcD[1][2]*MdcD[1][2]);

  delta[2] = sqrtf(MdcD[2][0]*MdcD[2][0]
		 + MdcD[2][1]*MdcD[2][1]
		 + MdcD[2][2]*MdcD[2][2]);

  for(i = 0; i < 3; i++) {
    for(j = 0; j < 3; j++) {
      Mdc[i*3+j] = MdcD[i][j]/delta[j];
    }
  }

  /* create the Pcrs_c */
  Pcrs_c[0] = vol->size[ROW]/2.0f;
  Pcrs_c[1] = vol->size[COL]/2.0f;
  Pcrs_c[2] = vol->size[SLICE]/2.0f;

  matrixMult3by1(MdcD,Pcrs_c,Pxyz_c);
  Pxyz_c[0]+=vol->vox2wrld[0][3];
  Pxyz_c[1]+=vol->vox2wrld[1][3];
  Pxyz_c[2]+=vol->vox2wrld[2][3];

  /* write the vars */
  writeBigEndianFloat32(fp,3,delta);
  writeBigEndianFloat32(fp,9,Mdc);
  writeBigEndianFloat32(fp,3,Pxyz_c);

  /* go to the end of the header space */
  unusedSpaceSize-=USED_SPACE_SIZE;
  fwrite(unusedArr,sizeof(char),unusedSpaceSize,fp);

  /* write voxels */
  writeBigEndianShort16(fp,numVox,vol->voxels);

  /* write mr parms */
  if(vol->mriParms != NULL) {
    writeBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->tr);
    writeBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->flipangle);
    writeBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->te);
    writeBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->ti);
    writeBigEndianFloat32(fp,1,&((mriParameters*)vol->mriParms)->fov);
  }

  fclose(fp);

  return VP_SUCCESS;
}

/**
 * loads a volume from one DICOM file with multiple frames
 * returns a volume struct
 */
volume *loadSingleDICOMVolume(char *filename) {
  notSupported();
  return NULL;
}

/**
 * writes a volume struct to a DICOM file with multiple frames
 * returns SUCCESS OR FAILURE
 */
int writeSingleDICOMVolume(volume *vol, char *filename) {
  return notSupported();
}

/**
 * loads a volume from one RAW volume file
 * returns a volume struct
 */
volume *loadRAWVolume(char *filename) {
  FILE *fp;
  unsigned int siz[3];
  volume *vol;

  /* open the file */
  fp = fopen(filename,"r");
  if(fp == NULL) {
    fprintf(stderr,"error: could not open file %s for reading an MGH volume\n",
	    filename);
    return NULL;
  }

  /* read the size */
  readBigEndianInt32(fp,3,siz);

  /* allocate the volume */
  vol = createVolume(1, siz[ROW], siz[COL], siz[SLICE], 1, 1);

  /* validate allocation */
  if(vol == NULL) {
    fprintf(stderr,"error: could not allocate volume voxels in loadRAWVolume.c\n");
    return NULL;
  }

  /* read the raw data */
  readDataIntoShortBuffer(fp,siz[ROW]*siz[COL]*siz[SLICE],vol->voxels);

  fclose(fp);

  return vol;
}

/**
 * writes a volume struct to a RAW volume file
 * returns SUCCESS OR FAILURE
 */
int writeRAWVolume(volume *vol, char *filename) {
  FILE *fp;

  /* validate the input */
  if(vol == NULL || vol->voxels == NULL || filename == NULL || filename[0] == '\0') {
    return VP_FAILURE;
  }

  /* open the file */
  fp = fopen(filename,"w");
  if(fp == NULL) {
    fprintf(stderr,"error: could not open file %s for writing an MGH volume\n",
	    filename);
    return VP_FAILURE;
  }

  /* write size */
  writeBigEndianInt32(fp,3,vol->size);

  /* write voxels */
  writeBigEndianShort16(fp,vol->size[ROW]*vol->size[COL]*vol->size[SLICE],
			vol->voxels);

  fclose(fp);

  return VP_SUCCESS;
}

/**
 * loads a volume from one RAWIV volume file
 * returns a volume struct
 */
volume *loadRAWIVVolume(char *filename) {
  FILE *fp;
  unsigned int siz[3];
  volume *vol;
  float rangeXYZ[6];
  unsigned int itrash[2];
  float ftrash[3];

  /* open the file */
  fp = fopen(filename,"r");
  if(fp == NULL) {
    fprintf(stderr,"error: could not open file %s for reading an MGH volume\n",
	    filename);
    return NULL;
  }

  /* read the ranges */
  readBigEndianFloat32(fp,6,rangeXYZ);

  /* read some trash */
  readBigEndianInt32(fp,2,itrash);

  /* read the size */
  readBigEndianInt32(fp,3,siz);

  /* allocate the volume */
  vol = createVolume(1,siz[ROW],siz[COL],siz[SLICE],1,1);

  /* validate allocation */
  if(vol == NULL) {
    fprintf(stderr,"error: could not allocate volume voxels in loadRAWVolume.c\n");
    return NULL;
  }

  /* read some more trash */
  readBigEndianFloat32(fp,3,ftrash);

  /* read the voxel sizes */
  readBigEndianFloat32(fp,1,&vol->vox2wrld[0][0]);
  readBigEndianFloat32(fp,1,&vol->vox2wrld[1][1]);
  readBigEndianFloat32(fp,1,&vol->vox2wrld[2][2]);

  /* use the ranges to fill in M */
  vol->vox2wrld[0][3] = rangeXYZ[0];
  vol->vox2wrld[1][3] = rangeXYZ[1];
  vol->vox2wrld[2][3] = rangeXYZ[2];
  vol->vox2wrld[3][3] = 1.0f;

  /* read the raw data and convert it, if necessary */
  readDataIntoShortBuffer(fp,siz[ROW]*siz[COL]*siz[SLICE],vol->voxels);

  fclose(fp);

  return vol;
}

/**
 * writes a volume struct to a RAWIV volume file
 * returns SUCCESS OR FAILURE
 */
int writeRAWIVVolume(volume *vol, char *filename) {
  FILE *fp;
  float maxRange[3];
  unsigned int siz;

  /* validate the input */
  if(vol == NULL || vol->voxels == NULL || filename == NULL || filename[0] == '\0') {
    return VP_FAILURE;
  }

  /* open the file */
  fp = fopen(filename,"w");
  if(fp == NULL) {
    fprintf(stderr,"error: could not open file %s for writing a RAWIV volume\n",filename);
    return VP_FAILURE;
  }

  /* write min range */
  writeBigEndianFloat32(fp,1,&vol->vox2wrld[0][3]);
  writeBigEndianFloat32(fp,1,&vol->vox2wrld[1][3]);
  writeBigEndianFloat32(fp,1,&vol->vox2wrld[2][3]);

  /* build max range */
  maxRange[0] = vol->vox2wrld[0][3] + vol->size[0]*vol->vox2wrld[0][0];
  maxRange[1] = vol->vox2wrld[1][3] + vol->size[1]*vol->vox2wrld[1][1];
  maxRange[2] = vol->vox2wrld[2][3] + vol->size[1]*vol->vox2wrld[2][2];

  /* write max range */
  writeBigEndianFloat32(fp,3,maxRange);

  /* write num vertices */
  siz = vol->size[0]*vol->size[1]*vol->size[2];
  writeBigEndianInt32(fp,1,&siz);

  /* write num cells */
  siz = (vol->size[0]-1)*(vol->size[1]-1)*(vol->size[2]-1);
  writeBigEndianInt32(fp,1,&siz);

  /* write size */
  writeBigEndianInt32(fp,3,vol->size);

  /* write origin */
  writeBigEndianFloat32(fp,1,&vol->vox2wrld[0][3]);
  writeBigEndianFloat32(fp,1,&vol->vox2wrld[1][3]);
  writeBigEndianFloat32(fp,1,&vol->vox2wrld[2][3]);

  /* write scale */
  writeBigEndianFloat32(fp,1,&vol->vox2wrld[0][0]);
  writeBigEndianFloat32(fp,1,&vol->vox2wrld[1][1]);
  writeBigEndianFloat32(fp,1,&vol->vox2wrld[2][2]);

  /* write voxels */
  writeBigEndianShort16(fp,vol->size[ROW]*vol->size[COL]*vol->size[SLICE],
			vol->voxels);

  fclose(fp);

  return VP_SUCCESS;
}

/**
 * loads a volume from one Nifti volume file
 * returns a volume struct
 */
volume *loadNiftiVolume(char *filename) {
  volume *vol = NULL;

//  nifti_image *nii;
//
//  // load the image
//  //  nii = nifti_image_read(filename, 1);
//  if(nii == NULL) {
//    fprintf(stderr,
//	    "error: could not open file %s for reading a Nifti volume\n",
//	    filename);
//    return NULL;
//  }
//
  // create a volume of the appropriate size (only support 4d images here) 
  //  vol = createVolume(nii->nt,nii->nx,nii->ny,nii->nz,1,1);

//  // convert to our datatype
//  switch(nii->datatype) {
//  case DT_UINT8:
//    break;
//  case DT_INT16:
//    memcpy(vol->voxels, nii->data, nii->nbyper*nii->nvox);
//    break;
//  case DT_INT32:
//    break;
//  case DT_FLOAT32:
//    break;
//  case DT_COMPLEX64:
//    break;
//  case DT_FLOAT64:
//    break;
//  }


  return vol;
}

/**
 * copy image pixels into the appropriate volume slice
 * ASSUMES THE VOLUME HAS 1 FRAME and 1 ECHO
 */
void copyPixels(volume* vol, image *img, int sliceIndex) {
  /* validate the input */
  if(vol == NULL || img == NULL || sliceIndex > vol->size[SLICE]
     || img->width != vol->size[COL] || img->height != vol->size[ROW]
     || img->numChannels != vol->size[CHANNEL]) {
    return;
  }

  /* copy pixels */
  memcpy(&vol->voxels[vol->size[ROW]*vol->size[COL]*vol->size[CHANNEL]
		      *sliceIndex],
	 img->pixels,
	 img->width*img->height*img->numChannels*sizeof(unsigned short));
}

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/volumeIO.c,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/

