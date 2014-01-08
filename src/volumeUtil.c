/*****************************************************************************
 * volumeUtil.c is the source file for the utility functions operating on
 * a volume struct for libvp
 * Oliver Hinds <oph@bu.edu> 2005-04-12
 *
 *
 *
 *****************************************************************************/

#define VOLUME_UTIL_VERSION_C "$Id: volumeUtil.c,v 1.15 2007/08/04 20:29:40 oph Exp $"

#include"volumeUtil.h"

/**
 * create a volume struct
 */
volume *createVolume(int frames, int rows, int cols,
                     int slices, int echos, int channels) {
  volume *vol = createEmptyVolume(frames, rows, cols, slices, echos, channels);

  /* validate allocation */
  if(vol == NULL) {
    return NULL;
  }

  /* allocate the voxels */
  vol->voxels = (unsigned short*) malloc(frames*rows*cols*slices*echos*channels
                                         *sizeof(unsigned short));

  /* validate allocation */
  if(vol->voxels == NULL) {
    vol->size[FRAME] = vol->size[ROW] = vol->size[COL] = vol->size[SLICE] = 0;
    freeVolume(vol);
    return NULL;
  }

  vol->tex3d = 0;

  return vol;
}

/**
 * create a volume struct from the header info of another volume
 */
volume *createVolumeFrom(volume *otherVol) {
  volume *vol = (volume*) malloc(sizeof(volume));

  /* validate allocation */
  if(vol == NULL) {
    return NULL;
  }

  /* assign relevant parameters */
  memcpy(vol->size,otherVol->size,sizeof(unsigned int)*_D);
  strcpy(vol->filename,otherVol->filename);
  vol->type = otherVol->type;
  vol->actualBPP = otherVol->actualBPP;

  /* calculate max and min dims */
  vol->minmaxSize[VP_MIN] = otherVol->minmaxSize[VP_MIN];
  vol->minmaxSize[VP_MAX] = otherVol->minmaxSize[VP_MAX];

  /* initialize coordinate transform matrix */
  memcpy(vol->vox2wrld,otherVol->vox2wrld,sizeof(float)*16);

  if(otherVol->mriParms == NULL) {
    vol->mriParms = NULL;
  }
  else {
    ((mriParameters*)vol->mriParms)->tr
        = ((mriParameters*)otherVol->mriParms)->tr;
    ((mriParameters*)vol->mriParms)->flipangle
        = ((mriParameters*)otherVol->mriParms)->flipangle;
    ((mriParameters*)vol->mriParms)->te
        = ((mriParameters*)otherVol->mriParms)->te;
    ((mriParameters*)vol->mriParms)->ti
        = ((mriParameters*)otherVol->mriParms)->ti;
    ((mriParameters*)vol->mriParms)->fov
        = ((mriParameters*)vol->mriParms)->fov;
  }

  vol->sliceDir = otherVol->sliceDir;

  memcpy(vol->textures,otherVol->textures,sizeof(unsigned int)*_D);
  memcpy(vol->selectedVoxel,otherVol->selectedVoxel,sizeof(int)*_D);

  /* image processing variable init */
  vol->brightnessAdjust = otherVol->brightnessAdjust;
  vol->contrastAdjust = otherVol->contrastAdjust;

  /* allocate the voxels */
  vol->voxels = (unsigned short*) malloc(vol->size[FRAME]*vol->size[ROW]
                                         *vol->size[COL]*vol->size[SLICE]
                                         *vol->size[ECHO]*vol->size[CHANNEL]
                                         *sizeof(unsigned short));

  /* validate allocation */
  if(vol->voxels == NULL) {
    vol->size[FRAME] = vol->size[ROW] = vol->size[COL] = vol->size[SLICE] = 0;
    freeVolume(vol);
    return NULL;
  }

  return vol;
}

/**
 * create a volume struct, but don't allocate the voxels
 * THIS SHOULD ONLY BE USED WHEN HEADER INFO IS BEING READ WITHOUT READING
 * VOXELS!
 */
volume *createEmptyVolume(int frames, int rows, int cols, int slices,
                          int echos, int channels) {
  int d;
  volume *vol = (volume*) malloc(sizeof(volume));

  /* validate allocation */
  if(vol == NULL) {
    return NULL;
  }

  /* assign relevant parameters */
  vol->size[FRAME] = frames;
  vol->size[ROW] = rows;
  vol->size[COL] = cols;
  vol->size[SLICE] = slices;
  vol->size[ECHO] = echos;
  vol->size[CHANNEL] = channels;
  vol->filename[0] = '\0';
  vol->type = SHORT;
  vol->actualBPP = 8*sizeof(unsigned short);

  /* calculate max and min dims */
  vol->minmaxSize[VP_MIN] = min(rows,min(cols,slices));
  vol->minmaxSize[VP_MAX] = max(rows,max(cols,slices));

  /* initial coordinate system info */
  vol->vox2wrld[0][0] = vol->vox2wrld[1][1] = vol->vox2wrld[2][2] = 1.0f;

  vol->vox2wrld[0][1] = vol->vox2wrld[0][2] = vol->vox2wrld[0][3]
      = vol->vox2wrld[1][0] = vol->vox2wrld[1][2] = vol->vox2wrld[1][3]
      = vol->vox2wrld[2][0] = vol->vox2wrld[2][1] = vol->vox2wrld[2][3]
      = vol->vox2wrld[3][0] = vol->vox2wrld[3][1] = vol->vox2wrld[3][2]
      = vol->vox2wrld[3][3] = 0.0f;

  vol->mriParms = NULL;
  vol->sliceDir = SLICE;

  vol->textures[FRAME] = vol->textures[ROW] = vol->textures[COL]
      = vol->textures[SLICE] = vol->textures[ECHO] = vol->textures[CHANNEL] = 0;
  for(d = 1; d < _D; d++) {
    vol->selectedVoxel[d] = vol->size[d]/2;
  }

  /* image processing variable init */
  vol->brightnessAdjust = 0.0f;
  vol->contrastAdjust = 1.0f;

  return vol;
}

/**
 * free a volume
 */
void freeVolume(volume *vol) {
  /* validate input */
  if(vol == NULL) return;

  /* free voxels */
  if(vol->size[FRAME] != 0 && vol->size[ROW] != 0 && vol->size[COL] != 0 && vol->size[SLICE] != 0) {
    free(vol->voxels);
  }

  free(vol);
  vol = NULL;
}

/**
 * slices a volume along a given dimension
 */
image *sliceVolume(volume *vol, int frame, int dim, int echo) {
  image *img;
  int dims[3];
  int i,j,k;
  unsigned short *vox = NULL;
  int numVox;

  getSliceDims(dim,dims);

  /* create the image */
  img = createImage(vol->size[dims[0]],vol->size[dims[1]],vol->size[CHANNEL]);

  /* validate the allocation */
  if(img == NULL) {
    return NULL;
  }

  numVox = 1;
  for(i = 0; i < _D; i++) {
    numVox *= vol->size[i];
  }

  /* copy the pixels */
  for(i = 0; i < vol->size[dims[0]]; i++) {
    for(j = 0; j < vol->size[dims[1]]; j++) {
      switch(dim) {
        case ROW:
          vox = &vol->voxels[frame*numVox/vol->size[FRAME] +
                             vol->size[ECHO]
                             *vol->size[CHANNEL]
                             *((j*vol->size[ROW]*vol->size[COL])
                               + i*vol->size[ROW] + vol->selectedVoxel[ROW])];
          break;
        case COL:
          vox = &vol->voxels[frame*numVox/vol->size[FRAME] +
                             vol->size[ECHO]
                             *vol->size[CHANNEL]
                             *((i*vol->size[ROW]*vol->size[COL])
                               + vol->selectedVoxel[COL]*vol->size[ROW] + j)];
          break;
        case SLICE:
          vox = &vol->voxels[frame*numVox/vol->size[FRAME] +
                             vol->size[ECHO]
                             *vol->size[CHANNEL]
                             *((vol->selectedVoxel[SLICE]
                                *vol->size[ROW]*vol->size[COL])
                               + j*vol->size[ROW] + i)];
          break;
      }
      for(k = 0; k < img->numChannels; k++) {
        img->pixels[img->numChannels*(j*vol->size[dims[0]]+i)+k]
            = *(vox+k*vol->size[CHANNEL]*vol->size[ECHO]);
      }
    }
  }

  return img;
}

/**
 * sets a slice of the volume along a given dimension
 */
/* int setSlice(volume *vol, int dim, int slice, unsigned short *voxels) { */
/*   int dims[3]; */
/*   int i,j; */
/*   unsigned short *vox = NULL; */

/*   /\* validate *\/ */
/*   if(vol == NULL || dim < 0 || dim > 2 || slice < 0 || slice > vol->size[dim] */
/*      || vox == NULL) { */
/*     return VP_FAILURE; */
/*   } */

/*   getSliceDims(dim,dims); */

/*   /\* copy the voxels *\/ */
/*   for(i = 0; i < vol->size[dims[0]]; i++) { */
/*     for(j = 0; j < vol->size[dims[1]]; j++) { */
/*       switch(dim) { */
/*       case ROW: */
/*   vox = &vol->voxels[vol->numChannels */
/*          *((j*vol->size[ROW]*vol->size[COL]) */
/*            + i*vol->size[ROW] + vol->selectedVoxel[ROW])]; */
/*   break; */
/*       case COL: */
/*   vox = &vol->voxels[vol->numChannels */
/*          *((i*vol->size[ROW]*vol->size[COL]) */
/*            + vol->selectedVoxel[COL]*vol->size[ROW] + j)]; */
/*   break; */
/*       case SLICE: */
/*   vox = &vol->voxels[vol->numChannels */
/*          *((vol->selectedVoxel[SLICE] */
/*             *vol->size[ROW]*vol->size[COL]) */
/*            + j*vol->size[ROW] + i)]; */
/*   break; */
/*       } */
/*       memcpy(vox, */
/*        &voxels[vol->numChannels*(j*vol->size[dims[0]]+i)], */
/*        sizeof(unsigned short)*vol->numChannels); */
/*     } */
/*   } */

/*   return VP_SUCCESS; */
/* } */

/**
 * load an array with the three linear voxel dimensions for row,col,slice
 */
void getVoxelSize(volume *vol, float voxelSizes[3]) {
  float one[4] = {1.0f,1.0f,1.0f,1.0f};
  float zero[4] = {0.0f,0.0f,0.0f,1.0f};
  float resultOne[4],resultZero[4];

  /* validate */
  if(vol == NULL) return;

  matrixMult4by1(vol->vox2wrld,one,resultOne);
  matrixMult4by1(vol->vox2wrld,zero,resultZero);

  voxelSizes[0] = fabsf(resultOne[0]-resultZero[0]);
  voxelSizes[1] = fabsf(resultOne[1]-resultZero[1]);
  voxelSizes[2] = fabsf(resultOne[2]-resultZero[2]);
}

/**
 * get the slice dimension order for a given slice direction
 */
void getSliceDims(int dir, int dims[3]) {
  switch(dir) {
    case ROW:
      dims[0] = COL;
      dims[1] = SLICE;
      dims[2] = ROW;
      break;
    case COL:
      dims[0] = SLICE;
      dims[1] = ROW;
      dims[2] = COL;
      break;
    case SLICE:
      dims[0] = ROW;
      dims[1] = COL;
      dims[2] = SLICE;
      break;
  }
}

/**
 * changes the current frame index, if possible
 * returns SUCCESS or FAILURE
 */
int changeCurFrameBy(volume *vol, int n) {
  /* validate input */
  if(vol == NULL) return VP_FAILURE;

  /* change the slice index */
  vol->selectedVoxel[FRAME] += n;

  /* validate the slice change */
  if(vol->selectedVoxel[FRAME] < 0) {
    vol->selectedVoxel[FRAME] = vol->size[FRAME] - 1;
  }
  if(vol->selectedVoxel[FRAME] >= vol->size[FRAME]) {
    vol->selectedVoxel[FRAME] = 0;
  }

  return VP_SUCCESS;
}

/**
 * changes the current slice index, if possible
 * returns SUCCESS or FAILURE
 */
int changeCurSliceBy(volume *vol, int n) {
  /* validate input */
  if(vol == NULL) return VP_FAILURE;

  /* change the slice index */
  vol->selectedVoxel[vol->sliceDir] += n;

  /* validate the slice change */
  if(vol->selectedVoxel[vol->sliceDir] < 0) {
    vol->selectedVoxel[vol->sliceDir] = 0;
  }
  if(vol->selectedVoxel[vol->sliceDir] >= vol->size[vol->sliceDir]) {
    vol->selectedVoxel[vol->sliceDir] = vol->size[vol->sliceDir] - 1;
  }

  return VP_SUCCESS;
}

/**
 * changes the current slice index to the first index, if possible
 * returns SUCCESS or FAILURE
 */
int goToFirstSlice(volume *vol) {
  /* validate input */
  if(vol == NULL) return VP_FAILURE;

  /* go to the first slice */
  vol->selectedVoxel[vol->sliceDir] = 0;

  return VP_SUCCESS;
}

/**
 * changes the current slice index to the last index, if possible
 * returns SUCCESS or FAILURE
 */
int goToLastSlice(volume *vol) {
  /* validate input */
  if(vol == NULL) return VP_FAILURE;

  /* go to the last slice */
  vol->selectedVoxel[vol->sliceDir] = vol->size[vol->sliceDir] - 1;

  return VP_SUCCESS;
}

/**
 * changes the current slice direction, if possible
 * returns SUCCESS or FAILURE
 */
int swapSliceDir(volume *vol, int dir) {
  /* validate input */
  if(vol == NULL) return VP_FAILURE;

  /* increment the slice dir */
  vol->sliceDir += dir;

  /* validate new slice dir */
  while(vol->sliceDir < ROW) {
    vol->sliceDir = SLICE;
  }
  while(vol->sliceDir > SLICE) {
    vol->sliceDir = ROW;
  }

  return VP_SUCCESS;
}

/**
 * increments the brightness adjustment for this volume
 */
int changeVolumeBrightness(volume *vol, float adj) {
  /* validate input */
  if(vol == NULL) return VP_FAILURE;

  vol->brightnessAdjust += adj;

  if(VP_DEBUG) {
    fprintf(stderr,"brightness adjust = %f\n", vol->brightnessAdjust);
  }

  return VP_SUCCESS;
}

/**
 * increments the contrast adjustment for this volume
 */
int changeVolumeContrast(volume *vol, float adj) {
  /* validate input */
  if(vol == NULL) return VP_FAILURE;

  vol->contrastAdjust += adj;

  if(VP_DEBUG) {
    fprintf(stderr,"contrast adjust = %f\n", vol->contrastAdjust);
  }

  return VP_SUCCESS;
}

/**
 * sets the initial volume contrast so the pizel of maximum value is
 * displayed at a given ratio of the maximum pixel intensity
 */
void setInitialVolumeContrast(volume *vol, float ratio) {
  int i,numVox = 1;
  unsigned short maxVal = 0;

  /* find the maximum voxel value */
  for(i = 0; i < _D; i++) numVox*=vol->size[i];
  for(i = 0; i < numVox; i++) {
    if(vol->voxels[i] > maxVal) maxVal = vol->voxels[i];
  }

  /* set the contrast */
  vol->contrastAdjust = ratio*USHRT_MAX/(float)maxVal;
}

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/volumeUtil.c,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
