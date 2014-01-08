/*****************************************************************************
 * imageIO.c is the source file for the misc image reading/writing functions
 * for libvp
 * Oliver Hinds <oph@bu.edu> 2005-04-12
 *
 *
 *
 *****************************************************************************/

#define IMAGE_IO_VERSION_C "$Id: imageIO.c,v 1.7 2007/05/22 19:18:06 oph Exp $"

#include<imageIO.h>

/**
 * read a dicom image by name
 * return a pointer to an image struct
 */
image *readDICOM(char* filename) {
  image *img;
  IMAGE *dicomImage;
  int numImages, numChannels;

  /* validate */
  if(filename == NULL) {
    return NULL;
  }

  /* read the dicom into an xmedcom IMAGE */
  if(dicom_read(filename,&dicomImage,&numImages,1,0)) {
    return NULL;
  }

  if(dicomImage->data.gray != NULL) {
    numChannels = 1;
  }
  else if(dicomImage->data.rgb != NULL) {
    numChannels = 3;
  }
  else {
    fprintf(stderr,"readDicom: error, unsupported number of color channels\n");
    return NULL;
  }

  /* convert into our datatype */
  img = createImage(dicomImage->w,dicomImage->h,numChannels);

  if(img == NULL) {
    return NULL;
  }

  /* copy the pixels */
  if(numChannels == 3) {
    bitDepthConvert8to16(dicomImage->data.rgb,img->pixels,
                         dicomImage->w*dicomImage->h*numChannels);
  }
  else {
    memcpy(img->pixels,dicomImage->data.gray,
           sizeof(unsigned short)*dicomImage->w*dicomImage->h);
  }

  dicom_free(dicomImage,1);

  return img;
}

/**
 * write an image struct to a dicom image file
 * return SUCCESS or FAILURE
 */
int writeDICOM(image *img, char *filename) {
  return notSupported();
}

/**
 * read a jpeg image by name
 * return a pointer to an image struct
 */
image *readJPEG(char* filename) {
  struct jpeg_decompress_struct dinfo;
  struct jpeg_error_mgr jerr;

  static int w, h, c;
  image *img = NULL;
  unsigned char *row, *buf;

  JSAMPARRAY pixels;

  int row_stride, i=0;

  FILE* infile = fopen(filename, "rb");
  if (!infile) {
    return NULL;
  }
  else {
    dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);

    jpeg_stdio_src(&dinfo, infile);
    jpeg_read_header(&dinfo, TRUE);
    jpeg_start_decompress(&dinfo);

    w = dinfo.output_width;
    h = dinfo.output_height;
    c = dinfo.output_components;

    row = buf = (unsigned char*) malloc(w*h*c*sizeof(unsigned char));
    if(buf == NULL) return NULL;

    row_stride = dinfo.output_width * dinfo.output_components;

    /* rgb comps (jpeg) */

    pixels = (*dinfo.mem->alloc_sarray) ((j_common_ptr) &dinfo,
                                         JPOOL_IMAGE, row_stride, 1);

    while(dinfo.output_scanline < dinfo.output_height) {
      jpeg_read_scanlines(&dinfo, pixels, 1);
      for(i=0; i<row_stride; i++) {
        row[i] = pixels[0][i];
      }
      row+=row_stride;
    }
    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);
    fclose(infile);
  }

  img = createImage(w,h,c);
  if(img == NULL) return NULL;

  /* convert to 16 bit */
  bitDepthConvert8to16(buf,img->pixels,w*h*c);
  free(buf);

  return img;
}

/**
 * write an image struct to a jpeg image file
 * return SUCCESS or FAILURE
 */
int writeJPEG(image *img, char *filename) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  int row_stride, w = img->width, h = img->height, siz;
  FILE* jfile = fopen(filename, "w+");
  unsigned char *buf;

  if(jfile == NULL) {
    fprintf(stderr,"error: couldn't open file %s to write a jpeg\n", filename);
    return VP_FAILURE;
  }

  if(img->pixels == NULL) {
    fprintf(stderr,"can't write a jpeg of a NULL image\n");
    return VP_FAILURE;
  }

  /* create a unsigned character version */
  siz = img->numChannels*w*h;
  buf = (unsigned char*) malloc(siz*sizeof(unsigned char));
  if(buf == NULL) {
    fprintf(stderr,"can't allocate for bit conversion, abort\n");
    return VP_FAILURE;
  }

  bitDepthConvert16to8(img->pixels,buf,siz);

  JSAMPROW row_pointer[1];

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  jpeg_stdio_dest(&cinfo, jfile);
  cinfo.image_width = w;
  cinfo.image_height = h;
  cinfo.input_components = img->numChannels;
  cinfo.in_color_space = JCS_RGB; /* arbitrary guess */
  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, 100, TRUE);

  jpeg_start_compress(&cinfo, TRUE);

  row_stride = cinfo.image_width * cinfo.input_components;

  while(cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = &buf[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  free(buf);

  return VP_SUCCESS;
}

/**
 * read a tiff image by name
 * return a pointer to an image struct
 */
image *readTIFF(char* filename) {
  //  /*Using Sam Leffler's libtiff library */
  //  TIFFRGBAImage img;
  //  image *im;
  //  uint32 *raster;
  //
  //  size_t npixels;
  //  int imgwidth, imgheight, imgcomponents;
  //
  //  TIFF *tif;
  //  char emsg[1024];
  //
  //  tif = TIFFOpen(filename, "r");
  //  if (tif == NULL){
  //    fprintf(stderr, "tif == NULL\n");
  //    return NULL;
  //  }
  //  if (TIFFRGBAImageBegin(&img, tif, 0,emsg)){
  //    npixels = img.width*img.height;
  //    raster = (uint32 *)_TIFFmalloc(npixels*sizeof(uint32));
  //    if (raster != NULL){
  //      if (TIFFRGBAImageGet(&img, raster, img.width, img.height) == 0){
  //  TIFFError(name, emsg);
  //  exit(1);
  //      }
  //    }
  //    TIFFRGBAImageEnd(&img);
  //  }
  //  else {
  //    TIFFError(name, emsg);
  //    return NULL;
  //  }
  //
  //  /* reverse from abgr to rgb */
  //  int i;
  //  im = createImage(img.width,img.height,3);
  //  for (i = 0; i < npixels; i++) {
  //    register unsigned char *cp = (unsigned char *) &raster[i];
  //
  //    im.pixels[i][0] = (unsigned short) cp[0];
  //
  //    t = cp[3];
  //    cp[3] = cp[0];
  //    cp[0] = t;
  //    t = cp[2];
  //    cp[2] = cp[1];
  //    cp[1] = t;
  //  }
  //
  //  return im;
  notSupported();
  return NULL;
}

/**
 * write an image struct to a tiff image file
 * return SUCCESS or FAILURE
 */
int writeTIFF(image *img, char *filename) {
  return notSupported();
}

/**
 * save a 8 bit per pixel pNm image from an image struct
 * return SUCCESS or FAILURE
 */
image *readPNM(char *filename) {
  notSupported();
  return NULL;
}

/**
 * save a 8 bit per pixel pNm image from a pixel array
 * numChannels can be 3 for color, 1 for greyscale
 * binary should be TRUE to write the image in binary format, FALSE for ASCII
 * return SUCCESS or FAILURE
 */
int writePNM(image *img, char *filename, int binary) {
  int i, x, y, c, counter = 0;

  /* validate the image */
  if(img->pixels == NULL) return VP_FAILURE;

  /* open the file for writing and validate */
  FILE *fp = fopen(filename, "w+");
  if(!fp) {
    fprintf(stderr,"error: could not open file %s for writing.\n", filename);
    return VP_FAILURE;
  }

  /* write the image based on whether its binary or not */
  if(binary) {

    /* print the pNm header info */
    if(img->numChannels == 3) {
      fprintf(fp,"P6\n");
    }
    else if(img->numChannels == 1) {
      fprintf(fp,"P5\n");
    }
    else {
      fprintf(stderr, "error: unsupported number of color channels=%d!\nsupported are 3 and 1.\n",
              img->numChannels);
      return VP_FAILURE;
    }

    fprintf(fp,"%d %d\n", img->width, img->height);

    switch(img->type) {
      case UCHAR:
        fprintf(fp,"%d\n",(int)pow(2,8*sizeof(unsigned char))-1);
        fclose(fp);

        /* reopen the file for binary writing */
        fp = fopen(filename, "ab" );
        fwrite(img->pixels, sizeof(unsigned char),
               img->numChannels*img->width*img->height, fp);
        break;
      case SHORT:
        fprintf(fp,"%d\n",(int)pow(2,8*sizeof(unsigned short))-1);
        fclose(fp);

        /* reopen the file for binary writing */
        fp = fopen(filename, "ab" );
        fwrite(img->pixels, sizeof(unsigned short),
               img->numChannels*img->width*img->height, fp);
        break;
      case INT:
      case LONG:
        fprintf(fp,"%d\n",(int)pow(2,8*sizeof(unsigned int))-1);
        fclose(fp);

        /* reopen the file for binary writing */
        fp = fopen(filename, "ab" );
        fwrite(img->pixels, sizeof(unsigned int),
               img->numChannels*img->width*img->height, fp);
        break;
      default:
        fprintf(stderr,"writePNM error: unsupported image type\n");
        return VP_FAILURE;
        break;
    }
  }
  else { /* ascii */
    /* write the header */
    if(img->numChannels == 3) {
      fprintf(fp,"P3\n");
    }
    else if(img->numChannels == 1) {
      fprintf(fp,"P2\n");
    }
    else {
      fprintf(stderr, "error: unsupported number of color channels=%d!\nsupported are 1 and 3.\n",
              img->numChannels);
      return VP_FAILURE;
    }

    fprintf(fp,"%d %d\n", img->width, img->height);

    switch(img->type) {
      case UCHAR:
        fprintf(fp,"%d\n",UCHAR_MAX);
        break;
      case SHORT:
        fprintf(fp,"%d\n",USHRT_MAX);
        break;
      case INT:
      case LONG:
        fprintf(fp,"%d\n",INT_MAX);
        break;
      default:
        fprintf(fp,"255\n");
        break;
    }

    /* write the pixel info */
    for(y = img->height-1; y >= 0; y--) {
      for(x = 0; x < img->width; x++) {
        i = (y*img->width + x) * img->numChannels;
        for(c = 0; c < img->numChannels; c++) {
          fprintf(fp, " %3d", img->pixels[i+c]);
          counter++;
          if(counter % 5 == 0)
            fprintf(fp, "\n");
        }
      }
    }
  }
  fclose(fp);

  return VP_SUCCESS;
}

/**
 * read a raw image by name
 * return a pointer to an image struct
 */
image *readDAT(char* filename) {
  unsigned int w,h,c;

  /* validate the image */
  if(filename == NULL) return NULL;

  /* open the file for reading and validate */
  FILE *fp = fopen(filename, "rb");
  if(!fp) {
    fprintf(stderr,"error: could not open file %s for reading.\n", filename);
    return NULL;
  }

  fread(&w, sizeof(unsigned int), 1, fp);
  fread(&h, sizeof(unsigned int), 1, fp);
  fread(&c, sizeof(unsigned int), 1, fp);

  image *img = createImage(w,h,c);
  fread(img->pixels,sizeof(unsigned int),w*h*c,fp);

  return img;
}

/**
 * write an image struct to a raw image file
 * return SUCCESS or FAILURE
 */
int writeDAT(image *img, char *filename) {
  return notSupported();
}



/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/imageIO.c,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
