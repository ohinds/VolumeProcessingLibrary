/*****************************************************************************
 * imageUtil.c is the source file for the misc image utility functions
 * for libvp
 * Oliver Hinds <oph@bu.edu> 2005-04-12
 *
 * 
 *
 *****************************************************************************/

#define IMAGE_UTIL_VERSION_C "$Id: imageUtil.c,v 1.9 2007/07/20 13:37:15 oph Exp $"

#include"imageUtil.h"

/**
 * create an image struct
 */
image *createImage(int width, int height, int numChannels) {
  image *img = (image*) malloc(sizeof(image));
  
  /* validate allocation */
  if(img == NULL) return NULL;

  /* assign attributes */
  img->width = width;
  img->height = height;
  img->numChannels = numChannels;
  img->type = SHORT;
  img->filename[0] = '\0';
  img->padX = img->padY = 0;

  /* initial coordinate system info */
  img->pix2wrld[0][0] = img->pix2wrld[1][1] = img->pix2wrld[2][2] = 1.0f;

  img->pix2wrld[0][1] = img->pix2wrld[0][2] = img->pix2wrld[0][3] 
    = img->pix2wrld[1][0] = img->pix2wrld[1][2] = img->pix2wrld[1][3] 
    = img->pix2wrld[2][0] = img->pix2wrld[2][1] = img->pix2wrld[2][3] 
    = img->pix2wrld[3][0] = img->pix2wrld[3][1] = img->pix2wrld[3][2] 
    = img->pix2wrld[3][3] = 0.0f;

  /* allocate pixels */
  img->pixels = (unsigned short*) malloc(img->numChannels
					 *img->width
					 *img->height
					 *sizeof(unsigned short));
  
  /* validate allocation */
  if(img->pixels == NULL) {
    img->width = 0;
    img->height = 0;
    freeImage(img);
    return NULL;
  }

  return img;
}

/**
 * free an image struct
 */
void freeImage(image *img) {
  /* validate input */
  if(img == NULL) return;

  /* free the pixels */
  if(img->width >0 && img->height > 0) {
    free(img->pixels);
  }

  /* free image struct */
  free(img);
}

/**
 * resizes an image using bilinear interpolation
 */
int resizeImageBilinear(image *img, int newW, int newH) {
  unsigned short *newPixels;
  int i,j,pixInd,k,m,n,numC,ul,ur,bl,br;
  double x,y,dx,dy,widthMag,heightMag;

  /* validate */
  if(img == NULL || img->pixels == NULL) {
    return VP_FAILURE;
  }
  numC = img->numChannels;
  widthMag = newW/(double)img->width;
  heightMag = newH/(double)img->height;

  newPixels = (unsigned short*) malloc(numC*newW*newH*sizeof(unsigned short));
  if(newPixels == NULL) {
    return VP_FAILURE;
  }

  /* assign the new pixels */
  for(i = 0; i < newH; i++) {
    y = i/heightMag;
    m = floor(y);
    dy = y-m;

    for(j = 0; j < newW; j++) {
      x = j/widthMag;
      n = floor(x);
      dx = x-n;

      ul = numC*(m*img->width+n);
      bl = numC*(((m==img->height-1)?img->height-1:m+1)*img->width+n);
      ur = numC*(m*img->width+((n==img->width-1)?img->width-1:n+1));
      br = numC*(((m==img->height-1)?img->height-1:m+1)*img->width+((n==img->width-1)?img->width-1:n+1));
		     
      pixInd = numC*(i*newW+j);
      /* get each component */
      for(k = 0; k < numC; k++) {	
	newPixels[pixInd+k] = img->pixels[ul+k] 
	  + dx*(img->pixels[ur+k]-img->pixels[ul+k])
	  + dy*(img->pixels[bl+k]-img->pixels[ul+k])
	  + dx*dy*(img->pixels[br+k]+img->pixels[ul+k]
		   -img->pixels[bl+k]-img->pixels[ur+k]);
      }
    }
  }
  
  /* save the new pixels and dims */
  free(img->pixels);
  img->pixels = newPixels;
  
  img->width = newW;
  img->height = newH;

  return VP_SUCCESS;
}
 
#define P(x) ((x > 0) ? x : 0)
/**
 * cubic spline weighting function
 */
double cubicWeighting(double x) {
  return (pow(P(x+2),3)-4*pow(P(x+1),3)+6*pow(P(x),3)-4*pow(P(x-1),3))/6.0;
}

/**
 * resizes an image using bicubic spline interpolation
 */
int resizeImageBicubic(image *img, int newW, int newH) {
  unsigned short *newPixels;
  int i,j,k,m,n,p,q,numC,pixInd,oldPixInd,localRow,localCol;
  double x,y,dx,dy,widthMag,heightMag;

  /* validate */
  if(img == NULL || img->pixels == NULL) {
    return VP_FAILURE;
  }
  numC = img->numChannels;
  widthMag = newW/(double)img->width;
  heightMag = newH/(double)img->height;

  /* allocate */
  newPixels = (unsigned short*) malloc(numC*newW*newH*sizeof(unsigned short));
  if(newPixels == NULL) {
    return VP_FAILURE;
  }

  /* assign the new pixels */
  for(i = 0; i < newH; i++) {
    y = i/heightMag;
    q = (int) floor(y);
    dy = y - q;

    for(j = 0; j < newW; j++) {
      x = j/widthMag;
      p = (int) floor(x);
      dx = x - p;

      pixInd = numC*(i*newW+j);      
      /* zero the color components */
      for(m = 0; m < numC; m++) {
	newPixels[pixInd+m] = 0;
      }

      /* iterate over the neigborhood */
      for(m = -1; m < 3; m++) {
	localRow = (q+m < 0) ? 0 : ((q+m >= img->height) ? img->height-1 : q+m);
	for(n = -1; n < 3; n++) {
	  localCol = (p+n < 0) ? 0 : ((p+n >= img->width) ? img->width-1 : p+n);

	  oldPixInd = numC*(localRow*img->width+localCol);
	  /* sum each color component */
	  for(k = 0; k < numC; k++) {
	    newPixels[pixInd+k] += cubicWeighting(m-dy) * cubicWeighting(n-dx) 
	      * img->pixels[oldPixInd+k];
	  }
	}
      }
    }
  }

  /* save the new pixels and dims */
  free(img->pixels);
  img->pixels = newPixels;

  img->width = newW;
  img->height = newH;

  return VP_SUCCESS;
}

/** image bit depth conversion functions **/

/**
 * bitDepthConvert a 32 bpp image to a 16 bpp image
 */
void bitDepthConvert32to16(unsigned int *buf32, unsigned short *buf16, 
			   int numPix) {
  int pix;
  float ratio;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    ratio = buf32[pix]/(float)UINT_MAX;
    buf16[pix] = (unsigned short) rint(ratio*USHRT_MAX);
  }
}

/**
 * bitDepthConvert a 16 bpp image to a 12 bpp image
 */
void bitDepthConvert16to32(unsigned short *buf16, unsigned int *buf32, 
			   int numPix) {
  int pix;
  float ratio;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    ratio = buf16[pix]/(float)USHRT_MAX;
    buf32[pix] = (unsigned int) rint(ratio*UINT_MAX);
  }
}

/**
 * bitDepthConvert a 32 bpp floating point image to a 16 bpp image
 */
void bitDepthConvert32fto16(float *buf32, unsigned short *buf16, 
			    int numPix) {
  int pix;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    if(buf32[pix] < 0) {
      buf16[pix] = 0;
    }
    else {
      buf16[pix] = (unsigned short) rintf(buf32[pix]);      
    }
  }
}

/**
 * bitDepthConvert a 16 bpp image 32 bpp floating point image
 */
void bitDepthConvert16to32f(unsigned short *buf16, float *buf32, 
			    int numPix) {
  int pix;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    buf32[pix] = (float)buf16[pix];
  }
}

/**
 * bitDepthConvert a 32 bpp image to a 8 bpp image
 */
void bitDepthConvert32to8(unsigned int *buf32, unsigned char *buf8, 
			  int numPix) {
  int pix;
  float ratio;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    ratio = buf32[pix]/(float)UINT_MAX;
    buf8[pix] = (unsigned char) rint(ratio*UCHAR_MAX);
  }
}

/**
 * bitDepthConvert a 8 bpp image to a 32 bpp image
 */
void bitDepthConvert8to32(unsigned char *buf8, unsigned int *buf32, 
			  int numPix) {
  int pix;
  float ratio;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    ratio = buf8[pix]/(float)UCHAR_MAX;
    buf32[pix] = (unsigned int) rint(ratio*UINT_MAX);
  }
}

/**
 * bitDepthConvert a 32 bpp floating point image to a 8 bpp image
 */
void bitDepthConvert32fto8(float *buf32, unsigned char *buf8, 
			   int numPix) {
  int pix;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    buf8[pix] = (unsigned char) rint(buf32[pix]*UCHAR_MAX);
  }
}

/**
 * bitDepthConvert a 8 bpp image to a 32 bpp floating point image
 */
void bitDepthConvert8to32f(unsigned char *buf8, float *buf32, 
			   int numPix) {
  int pix;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    buf32[pix] = buf8[pix]/(float)UCHAR_MAX;
  }
}

/**
 * bitDepthConvert a 16 bpp image to a 8 bpp image
 */
void bitDepthConvert16to8(unsigned short *buf16, unsigned char *buf8, 
			  int numPix) {
  int pix;
  float ratio;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    ratio = buf16[pix]/(float)USHRT_MAX;
    buf8[pix] = (unsigned char) rint(ratio*UCHAR_MAX);
  }
}

/**
 * bitDepthConvert a 8 bpp image to a 16 bpp image
 */
void bitDepthConvert8to16(unsigned char *buf8, unsigned short *buf16, 
			  int numPix) {
  int pix;
  float ratio;

  /* iterate over each pixel, rescaling the values */
  for(pix = 0; pix < numPix; pix++) {
    ratio = buf8[pix]/(float)UCHAR_MAX;
    buf16[pix] = (unsigned short) rint(ratio*USHRT_MAX);
  }
}

/**
 * increments the brightness adjustment for this image
 */
int changeImageBrightness(image *img, float adj) {
  /* validate input */
  if(img == NULL) return VP_FAILURE;

  img->brightnessAdjust += adj;

  if(VP_DEBUG) {
    fprintf(stderr,"brightness adjust = %f\n", img->brightnessAdjust);
  }

  return VP_SUCCESS;
}

/**
 * increments the contrast adjustment for this image
 */
int changeImageContrast(image *img, float adj) {
  /* validate input */
  if(img == NULL) return VP_FAILURE;

  img->contrastAdjust += adj;

  if(VP_DEBUG) {
    fprintf(stderr,"contrast adjust = %f\n", img->contrastAdjust);
  }

  return VP_SUCCESS;
}

/**
 * checks if two images are "compatible" 
 */
int imagesCompatible(image *img1, image *img2) {

  return img1 != NULL && img2 != NULL 
    && img1->height == img2->height 
    && img1->width == img2->width
    && img1->numChannels == img2->numChannels
    && img1->type == img2->type;
}

/**
 * subtracts two images
 */
image *subtractImages(image *img1, image *img2) {
  image *subimg;
  int i;

  if(!imagesCompatible(img1,img2)) {
    return NULL;
  }


  // subtract
  subimg = createImage(img1->width, img1->height, img1->numChannels);
  for(i = 0; i < img1->height*img1->width*img1->numChannels; i++) {
    subimg->pixels[i] =  
      (img2->pixels[i] > img1->pixels[i]) ? 
      img2->pixels[i]-img1->pixels[i] : img1->pixels[i]-img2->pixels[i];
  }

  return subimg;
}

/**
 * flip an image
 * reverse ordering of rows in buffer, since jpeg storage is flipped
 * with respect to openGL's buffer format.
 */
void flipImage(image *img) {
  int row_ind, col_ind, nRows, nCols,
    w = img->width, h = img->height, c = img->numChannels;
  unsigned short *imgbuf = img->pixels, *flip = NULL;

  /* check for proper bpp */
  

  /* each row contains 'w' pixels with c color components each */
  nCols = w * c;
  nRows = h;

  if((flip=(unsigned short *) malloc((size_t) w * h * c * sizeof(unsigned short))) == NULL)
    fprintf(stderr, "flipImage: error allocating buffer space\n");

  for(row_ind = 0; row_ind < nRows; row_ind++) {
    for(col_ind = 0; col_ind < nCols; col_ind++) {
      /* copy each row to the its vertical complement */
      flip[row_ind*nCols + col_ind]
	= imgbuf[(nRows-row_ind-1)*nCols + col_ind];
    }
  }
  free(imgbuf);
  img->pixels = flip;

} 

/**
 * flattens an image by ignoring the alpha channel
 */
void flattenImage(image* img) {
  int i,j;
  int c = img->numChannels;

  unsigned short *imgbuf = (unsigned short*) malloc(c*img->width*img->height*sizeof(unsigned short));

  for(i = 0; i < img->height; i++) {
    for(j = 0; j < img->width; j++) {
      imgbuf[c*(img->width*i+j)] = img->pixels[4*(img->width*i+j)];
      imgbuf[c*(img->width*i+j)+1] = img->pixels[4*(img->width*i+j)+1];
      imgbuf[c*(img->width*i+j)+2] = img->pixels[4*(img->width*i+j)+2];
    }
  }

  free(img->pixels);
  img->pixels = imgbuf;
}

/**
 * pads an image so dimensions power of 2, doesn't change w and h, but
 * gives the number of pixels added in each dimension
 */
void padImagePow2(image* img) {
  int i,j,
      newX = pow(2,ceil(log(img->width)/log(2.0))),
      newY = pow(2,ceil(log(img->height)/log(2.0)));
  int c = img->numChannels;

  /* test for no action necessary */
  if(newX == img->width && newY == img->height) {
    img->padX = img->padY = 0;
    return;
  }

  unsigned short *imgbuf = (unsigned short*) malloc(c*newX*newY*sizeof(unsigned short));

  for(i = 0; i < newY; i++) {
    for(j = 0; j < newX; j++) {
      if(i < img->height && j < img->width) {
	imgbuf[c*(newX*i+j)] = img->pixels[c*(img->width*i+j)];
	imgbuf[c*(newX*i+j)+1] = img->pixels[c*(img->width*i+j)+1];
	imgbuf[c*(newX*i+j)+2] = img->pixels[c*(img->width*i+j)+2];
      }
      else {
	imgbuf[c*(newX*i+j)] = 0;
	imgbuf[c*(newX*i+j)+1] = 0;
	imgbuf[c*(newX*i+j)+2] = 0;
      }
    }
  }

  img->padX = newX - img->width;
  img->padY = newY - img->height;

  free(img->pixels);
  img->pixels = imgbuf;
}

/**
 * trim the image down to the size in the image struct from the size specified
 */
void trimImage(image* img, int pixAddX) {
  int i,j;
  int c = img->numChannels;
  unsigned short *imgbuf = (unsigned short*)
    malloc(c*img->width*img->height*sizeof(unsigned short));

  for(i = 0; i < img->height; i++) {
    for(j = 0; j < img->width; j++) {
      imgbuf[c*((img->width+pixAddX)*i+j)] = img->pixels[c*(img->width*i+j)];
      imgbuf[c*((img->width+pixAddX)*i+j)+1]= img->pixels[c*(img->width*i+j)+1];
      imgbuf[c*((img->width+pixAddX)*i+j)+2]= img->pixels[c*(img->width*i+j)+2];
    }
  }

  free(img->pixels);
  img->pixels = imgbuf;
}


/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/imageUtil.c,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/

