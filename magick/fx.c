/*
% Copyright (C) 2003 GraphicsMagick Group
% Copyright (C) 2002 ImageMagick Studio
% Copyright 1991-1999 E. I. du Pont de Nemours and Company
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                 FFFFF  X   X                                %
%                                 F       X X                                 %
%                                 FFF      X                                  %
%                                 F       X X                                 %
%                                 F      X   X                                %
%                                                                             %
%                                                                             %
%                 GraphicsMagick Image Special Effects Methods                %
%                                                                             %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                 October 1996                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/color.h"
#include "magick/effect.h"
#include "magick/enhance.h"
#include "magick/fx.h"
#include "magick/gem.h"
#include "magick/log.h"
#include "magick/pixel_cache.h"
#include "magick/pixel_iterator.h"
#include "magick/monitor.h"
#include "magick/resize.h"
#include "magick/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     C h a r c o a l I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method CharcoalImage creates a new image that is a copy of an existing
%  one with the edge highlighted.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  The format of the CharcoalImage method is:
%
%      Image *CharcoalImage(const Image *image,const double radius,
%        const double sigma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o charcoal_image: Method CharcoalImage returns a pointer to the image
%      after it is embossed.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The image.
%
%    o radius: the radius of the pixel neighborhood.
%
%    o sigma: The standard deviation of the Gaussian, in pixels.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
MagickExport Image *CharcoalImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
  Image
    *blur_image,
    *charcoal_image,
    *edge_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  charcoal_image=CloneImage(image,0,0,True,exception);
  if (charcoal_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageType(charcoal_image,GrayscaleType);
  edge_image=EdgeImage(charcoal_image,radius,exception);
  if (edge_image == (Image *) NULL)
    return((Image *) NULL);
  DestroyImage(charcoal_image);
  blur_image=BlurImage(edge_image,radius,sigma,exception);
  if (blur_image == (Image *) NULL)
    return((Image *) NULL);
  DestroyImage(edge_image);
  (void) NormalizeImage(blur_image);
  (void) NegateImage(blur_image,False);
  (void) SetImageType(blur_image,GrayscaleType);
  return(blur_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     C o l o r i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ColorizeImage() blends the fill color with each pixel in the image.
%  A percentage blend is specified with opacity.  Control the application
%  of different color components by specifying a different percentage for
%  each component (e.g. 90/100/10 is 90% red, 100% green, and 10% blue).
%
%  The format of the ColorizeImage method is:
%
%      Image *ColorizeImage(const Image *image,const char *opacity,
%        const PixelPacket target,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o opacity:  A character string indicating the level of opacity as a
%      percentage.
%
%    o target: A color value.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
typedef struct _ColorizeImagePixelsOptions
{
  DoublePixelPacket amount;
  DoublePixelPacket color;
} ColorizeImagePixelsOptions;
static MagickPassFail
ColorizeImagePixelsCB(void *mutable_data,                /* User provided mutable data */
                      const void *immutable_data,        /* User provided immutable data */
                      const Image *source_image,         /* Source image */
                      const PixelPacket *source_pixels,  /* Pixel row in source image */
                      const IndexPacket *source_indexes, /* Pixel row indexes in source image */
                      Image *new_image,                  /* New image */
                      PixelPacket *new_pixels,           /* Pixel row in new image */
                      IndexPacket *new_indexes,          /* Pixel row indexes in new image */
                      const long npixels,                /* Number of pixels in row */
                      ExceptionInfo *exception           /* Exception report */
                      )
{
  ColorizeImagePixelsOptions
    options = *((const ColorizeImagePixelsOptions *) immutable_data);

  register long
    i;

  ARG_NOT_USED(mutable_data);
  ARG_NOT_USED(source_image);
  ARG_NOT_USED(source_indexes);
  ARG_NOT_USED(new_image);
  ARG_NOT_USED(new_indexes);
  ARG_NOT_USED(exception);

  for (i=0; i < npixels; i++)
    {
      new_pixels[i].red=(Quantum)
        ((source_pixels[i].red*(100.0-options.amount.red)+
          options.color.red*options.amount.red)/100.0);
      new_pixels[i].green=(Quantum)
        ((source_pixels[i].green*(100.0-options.amount.green)+
          options.color.green*options.amount.green)/100.0);
      new_pixels[i].blue=(Quantum)
        ((source_pixels[i].blue*(100.0-options.amount.blue)+
          options.color.blue*options.amount.blue)/100.0);
      new_pixels[i].opacity=(Quantum)
        ((source_pixels[i].opacity*(100.0-options.amount.opacity)+
          options.color.opacity*options.amount.opacity)/100.0);
    }

  return MagickPass;
}
MagickExport Image *ColorizeImage(const Image *image,const char *opacity,
  const PixelPacket target,ExceptionInfo *exception)
{
#define ColorizeImageText  "Colorize the image...  "

  ColorizeImagePixelsOptions
    options;

  Image
    *colorize_image;

  long
    count;

  unsigned int
    is_grayscale;

  /*
    Allocate colorized image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  is_grayscale=image->is_grayscale;
  colorize_image=CloneImage(image,image->columns,image->rows,True,exception);
  if (colorize_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageType(colorize_image,TrueColorType);
  if (opacity == (const char *) NULL)
    return(colorize_image);
  /*
    Determine percentage RGB values of the pen color.
  */
  options.amount.red=100.0;
  options.amount.green=100.0;
  options.amount.blue=100.0;
  options.amount.opacity=0.0;
  count=sscanf(opacity,"%lf%*[/,]%lf%*[/,]%lf%*[/,]%lf",
    &options.amount.red,&options.amount.green,&options.amount.blue,&options.amount.opacity);
  if (count == 1)
    {
      if (options.amount.red == 0.0)
        return(colorize_image);
      options.amount.green=options.amount.red;
      options.amount.blue=options.amount.red;
      options.amount.opacity=options.amount.red;
    }
  options.color.red=target.red;
  options.color.green=target.green;
  options.color.blue=target.blue;
  options.color.opacity=target.opacity;
  /*
    Colorize DirectClass image.
  */
  (void) PixelIterateDualNew(ColorizeImagePixelsCB,NULL,
                             ColorizeImageText,NULL,&options,
                             image->columns,image->rows,image,0,0,
                             colorize_image,0,0,&colorize_image->exception);
  colorize_image->is_grayscale=(is_grayscale && IsGray(target));
  return(colorize_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     C o n v o l v e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvolveImage() applies a custom convolution kernel to the image.
%
%  The format of the ConvolveImage method is:
%
%      Image *ConvolveImage(const Image *image,const unsigned int order,
%        const double *kernel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o order: The number of columns and rows in the filter kernel.
%
%    o kernel: An array of double representing the convolution kernel.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
#define ConvolveImageText  "Convolving image...  "
MagickExport Image *ConvolveImage(const Image *image,const unsigned int order,
                                  const double *kernel,ExceptionInfo *exception)
{
  double
    *normal_kernel;

  Image
    *convolve_image;

  long
    width,
    y;

  volatile MagickPassFail
    status;

  /*
    Initialize convolve image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=(long) order;
  if ((width % 2) == 0)
    ThrowImageException3(OptionError,UnableToConvolveImage,
                         KernelWidthMustBeAnOddNumber);
  if (((long) image->columns < width) || ((long) image->rows < width))
    ThrowImageException3(OptionError,UnableToConvolveImage,
                         ImageSmallerThanKernelWidth);
  convolve_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (convolve_image == (Image *) NULL)
    return((Image *) NULL);
  convolve_image->storage_class=DirectClass;
  {
    /*
      Build normalized kernel.
    */
    double
      normalize;
    
    register long
      i;

    normal_kernel=MagickAllocateMemory(double *,width*width*sizeof(double));
    if (normal_kernel == (double *) NULL)
      {
        DestroyImage(convolve_image);
        ThrowImageException(ResourceLimitError,MemoryAllocationFailed,
                            MagickMsg(OptionError,UnableToConvolveImage));
      }
    normalize=0.0;
    for (i=0; i < (width*width); i++)
      normalize+=kernel[i];
    if (AbsoluteValue(normalize) <= MagickEpsilon)
      normalize=1.0;
    normalize=1.0/normalize;
    for (i=0; i < (width*width); i++)
      {
        normal_kernel[i]=normalize*kernel[i];
      }
  }
  
  if (LogMagickEvent(TransformEvent,GetMagickModule(),
                     "  ConvolveImage with %ldx%ld kernel:",width,width))
    {
      /*
        Log convolution matrix.
      */
      char
        cell_text[13],
        row_text[65];

      const double
        *k;

      long
        u,
        v;

      k=kernel;
      for (v=0; v < width; v++)
        {
          *row_text='\0';
          for (u=0; u < width; u++)
            {
              FormatString(cell_text,"%#12.4g",*k++);
              (void) strlcat(row_text,cell_text,sizeof(cell_text));
              if (u%5 == 4)
                {
                  (void) LogMagickEvent(TransformEvent,GetMagickModule(),
                                        "   %.64s", row_text);
                  *row_text='\0';
                }
            }
          if (u > 5)
            (void) strlcat(row_text,"\n",sizeof(row_text));
          if (row_text[0] != '\0')
            (void) LogMagickEvent(TransformEvent,GetMagickModule(),
                                  "   %.64s", row_text);
        }
    }

  status=MagickPass;
  /*
    Convolve image.
  */
  {
    unsigned long
      row_count=0;

    ThreadViewSet
      *read_view_set,
      *write_view_set;

    DoublePixelPacket
      zero;

    read_view_set=AllocateThreadViewSet((Image *) image,exception);
    write_view_set=AllocateThreadViewSet(convolve_image,exception);
    if ((read_view_set == (ThreadViewSet *) NULL) ||
        (write_view_set == (ThreadViewSet *) NULL))
      {
        DestroyThreadViewSet(read_view_set);
        DestroyThreadViewSet(write_view_set);
        MagickFreeMemory(normal_kernel);
        DestroyImage(convolve_image);
        return (Image *) NULL;
      }

    (void) memset(&zero,0,sizeof(DoublePixelPacket));
#pragma omp parallel for schedule(static,64)
    for (y=0; y < (long) convolve_image->rows; y++)
      {
        ViewInfo
          *read_view,
          *write_view;

        const PixelPacket
          *p;
    
        PixelPacket
          *q;

        long
          x;

        MagickBool
          thread_status;

        thread_status=status;
        if (thread_status == MagickFail)
          continue;

        read_view=AccessThreadView(read_view_set);
        p=AcquireCacheView(read_view,-width/2,y-width/2,image->columns+width,width,
                           exception);
        write_view=AccessThreadView(write_view_set);
        q=SetCacheView(write_view,0,y,convolve_image->columns,1);
        if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          thread_status=MagickFail;

        if (thread_status != MagickFail)
          {
            for (x=0; x < (long) convolve_image->columns; x++)
              {
                DoublePixelPacket
                  pixel;

                const PixelPacket
                  *r;

                long
                  u,
                  v;

                const double
                  *k;

                r=p;
                pixel=zero;
                k=normal_kernel;
                for (v=width; v > 0; v--)
                  {
                    for (u=0; u < width; u++)
                      {
                        pixel.red+=(*k)*r[u].red;
                        pixel.green+=(*k)*r[u].green;
                        pixel.blue+=(*k)*r[u].blue;
                        pixel.opacity+=(*k)*r[u].opacity;
                        k++;
                      }
                    r+=image->columns+width;
                  }
                q->red=RoundDoubleToQuantum(pixel.red);
                q->green=RoundDoubleToQuantum(pixel.green);
                q->blue=RoundDoubleToQuantum(pixel.blue);
                q->opacity=RoundDoubleToQuantum(pixel.opacity);
                p++;
                q++;
              }
            if (!SyncCacheView(write_view))
              {
                thread_status=MagickFail;
                CopyException(exception,&convolve_image->exception);
              }
          }
#pragma omp critical
        {
          row_count++;
          if (QuantumTick(row_count,image->rows))
            if (!MagickMonitor(ConvolveImageText,row_count,image->rows,exception))
              thread_status=MagickFail;
          
          if (thread_status == MagickFail)
            status=MagickFail;
        }
      }
    DestroyThreadViewSet(write_view_set);
    DestroyThreadViewSet(read_view_set);
  }
  MagickFreeMemory(normal_kernel);
  if (MagickFail == status)
    {
      DestroyImage(convolve_image);
      convolve_image=(Image *) NULL;
    }
  else
    {
      convolve_image->is_grayscale=image->is_grayscale;
    }
  return(convolve_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     I m p l o d e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ImplodeImage creates a new image that is a copy of an existing
%  one with the image pixels "implode" by the specified percentage.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ImplodeImage method is:
%
%      Image *ImplodeImage(const Image *image,const double amount,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o implode_image: Method ImplodeImage returns a pointer to the image
%      after it is implode.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The image.
%
%    o amount:  Define the extent of the implosion.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
MagickExport Image *ImplodeImage(const Image *image,const double amount,
                                 ExceptionInfo *exception)
{
#define ImplodeImageText  "Implode image...  "

  double
    distance,
    radius,
    x_center,
    x_distance,
    x_scale,
    y_center,
    y_distance,
    y_scale;

  Image
    *implode_image;

  long
    y;

  register long
    x;

  register PixelPacket
    *q;

  /*
    Initialize implode image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  implode_image=CloneImage(image,image->columns,image->rows,True,exception);
  if (implode_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageType(implode_image,implode_image->background_color.opacity !=
    OpaqueOpacity ? TrueColorMatteType : TrueColorType);
  /*
    Compute scaling factor.
  */
  x_scale=1.0;
  y_scale=1.0;
  x_center=0.5*image->columns;
  y_center=0.5*image->rows;
  radius=x_center;
  if (image->columns > image->rows)
    y_scale=(double) image->columns/image->rows;
  else
    if (image->columns < image->rows)
      {
        x_scale=(double) image->rows/image->columns;
        radius=y_center;
      }
  /*
    Implode each row.
  */
  for (y=0; y < (long) image->rows; y++)
  {
    q=SetImagePixels(implode_image,0,y,implode_image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    y_distance=y_scale*(y-y_center);
    for (x=0; x < (long) image->columns; x++)
    {
      /*
        Determine if the pixel is within an ellipse.
      */
      x_distance=x_scale*(x-x_center);
      distance=x_distance*x_distance+y_distance*y_distance;
      if (distance >= (radius*radius))
        *q=AcquireOnePixel(image,x,y,exception);
      else
        {
          double
            factor;

          /*
            Implode the pixel.
          */
          factor=1.0;
          if (distance > 0.0)
            factor=pow(sin(MagickPI*sqrt(distance)/radius/2),-amount);
          *q=InterpolateColor(image,factor*x_distance/x_scale+x_center,
            factor*y_distance/y_scale+y_center,exception);
        }
      q++;
    }
    if (!SyncImagePixels(implode_image))
      break;
    if (QuantumTick(y,image->rows))
      if (!MagickMonitor(ImplodeImageText,y,image->rows,exception))
        break;
  }
  implode_image->is_grayscale=image->is_grayscale;
  return(implode_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     M o r p h I m a g e s                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The MorphImages() method requires a minimum of two images.  The first
%  image is transformed into the second by a number of intervening images
%  as specified by frames.
%
%  The format of the MorphImage method is:
%
%      Image *MorphImages(const Image *image,const unsigned long number_frames,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o number_frames:  Define the number of in-between image to generate.
%      The more in-between frames, the smoother the morph.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
typedef struct _MorphImagePixelsOptions
{
  double alpha;
  double beta;
} MorphImagePixelsOptions;
static MagickPassFail
MorphImagePixelsCB(void *mutable_data,                /* User provided mutable data */
                   const void *immutable_data,        /* User provided immutable data */
                   const Image *source_image,         /* Source image */
                   const PixelPacket *source_pixels,  /* Pixel row in source image */
                   const IndexPacket *source_indexes, /* Pixel row indexes in source image */
                   Image *new_image,                  /* New image */
                   PixelPacket *new_pixels,           /* Pixel row in new image */
                   IndexPacket *new_indexes,          /* Pixel row indexes in new image */
                   const long npixels,                /* Number of pixels in row */
                   ExceptionInfo *exception           /* Exception report */
                   )
{
  MorphImagePixelsOptions
    options = *((const MorphImagePixelsOptions *) immutable_data);

  register long
    i;

  ARG_NOT_USED(mutable_data);
  ARG_NOT_USED(source_image);
  ARG_NOT_USED(source_indexes);
  ARG_NOT_USED(new_image);
  ARG_NOT_USED(new_indexes);
  ARG_NOT_USED(exception);

  for (i=0; i < npixels; i++)
    {
      new_pixels[i].red=(Quantum) (options.alpha*new_pixels[i].red+
                                   options.beta*source_pixels[i].red+0.5);
      new_pixels[i].green=(Quantum) (options.alpha*new_pixels[i].green+
                                     options.beta*source_pixels[i].green+0.5);
      new_pixels[i].blue=(Quantum) (options.alpha*new_pixels[i].blue+
                                    options.beta*source_pixels[i].blue+0.5);
      new_pixels[i].opacity=(Quantum) (options.alpha*new_pixels[i].opacity+
                                       options.beta*source_pixels[i].opacity+0.5);
    }

  return MagickPass;
}
MagickExport Image *MorphImages(const Image *image,
  const unsigned long number_frames,ExceptionInfo *exception)
{
#define MorphImageText  "Morph image sequence...  "

  MorphImagePixelsOptions
    options;

  Image
    *clone_image,
    *morph_image,
    *morph_images;

  MonitorHandler
    handler;

  register const Image
    *next;

  register long
    i;

  unsigned long
    scene;

  /*
    Clone first frame in sequence.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  morph_images=CloneImage(image,0,0,True,exception);
  if (morph_images == (Image *) NULL)
    return((Image *) NULL);
  if (image->next == (Image *) NULL)
    {
      /*
        Morph single image.
      */
      for (i=1; i < (long) number_frames; i++)
      {
        morph_images->next=CloneImage(image,0,0,True,exception);
        if (morph_images->next == (Image *) NULL)
          {
            DestroyImageList(morph_images);
            return((Image *) NULL);
          }
        morph_images->next->previous=morph_images;
        morph_images=morph_images->next;
        if (!MagickMonitor(MorphImageText,i,number_frames,exception))
          break;
      }
      while (morph_images->previous != (Image *) NULL)
        morph_images=morph_images->previous;
      return(morph_images);
    }
  /*
    Morph image sequence.
  */
  scene=0;
  for (next=image; next->next != (Image *) NULL; next=next->next)
  {
    handler=SetMonitorHandler((MonitorHandler) NULL);
    for (i=0; i < (long) number_frames; i++)
    {
      options.beta=((double) i+1.0)/(number_frames+1.0);
      options.alpha=1.0-options.beta;
      clone_image=CloneImage(next,0,0,True,exception);
      if (clone_image == (Image *) NULL)
        break;
      morph_images->next=ZoomImage(clone_image,
        (unsigned long) (options.alpha*next->columns+options.beta*next->next->columns+0.5),
        (unsigned long) (options.alpha*next->rows+options.beta*next->next->rows+0.5),exception);
      DestroyImage(clone_image);
      if (morph_images->next == (Image *) NULL)
        break;
      morph_images->next->previous=morph_images;
      morph_images=morph_images->next;
      clone_image=CloneImage(next->next,0,0,True,exception);
      if (clone_image == (Image *) NULL)
        break;
      morph_image=ZoomImage(clone_image,morph_images->columns,
        morph_images->rows,exception);
      DestroyImage(clone_image);
      if (morph_image == (Image *) NULL)
        break;
      (void) SetImageType(morph_images,TrueColorType);
      (void) PixelIterateDualNew(MorphImagePixelsCB,NULL,
                                 MorphImageText,NULL,&options,
                                 morph_images->columns,morph_images->rows,morph_image,0,0,
                                 morph_images,0,0,exception);
      DestroyImage(morph_image);
    }
    if (i < (long) number_frames)
      break;
    /*
      Clone last frame in sequence.
    */
    morph_images->next=CloneImage(next->next,0,0,True,exception);
    if (morph_images->next == (Image *) NULL)
      break;
    morph_images->next->previous=morph_images;
    morph_images=morph_images->next;
    (void) SetMonitorHandler(handler);
    if (!MagickMonitor(MorphImageText,scene,GetImageListLength(image),exception))
      break;
    scene++;
  }
  while (morph_images->previous != (Image *) NULL)
    morph_images=morph_images->previous;
  if (next->next != (Image *) NULL)
    {
      DestroyImageList(morph_images);
      return((Image *) NULL);
    }
  return(morph_images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     O i l P a i n t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OilPaintImage() applies a special effect filter that simulates an oil
%  painting.  Each pixel is replaced by the most frequent color occurring
%  in a circular region defined by radius.
%
%  The format of the OilPaintImage method is:
%
%      Image *OilPaintImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o radius: The radius of the circular neighborhood.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
#define PaintHistSize 256
MagickExport Image *OilPaintImage(const Image *image,const double radius,
                                  ExceptionInfo *exception)
{
#define OilPaintImageTag  "OilPaint/Image"

  Image
    *paint_image;

  long
    width,
    y;

  unsigned long
    row_count=0;
  
  ThreadViewSet
    *read_view_set,
    *write_view_set;

  volatile MagickPassFail
    status=MagickPass;

  /*
    Initialize painted image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth(radius,0.5);
  if (((long) image->columns < width) || ((long) image->rows < width))
    ThrowImageException3(OptionError,UnableToPaintImage,
                         ImageSmallerThanRadius);

  paint_image=CloneImage(image,image->columns,image->rows,True,exception);
  if (paint_image == (Image *) NULL)
    return((Image *) NULL);

  read_view_set=AllocateThreadViewSet((Image *) image,exception);
  write_view_set=AllocateThreadViewSet(paint_image,exception);
  if ((read_view_set == (ThreadViewSet *) NULL) ||
      (write_view_set == (ThreadViewSet *) NULL))
    {
      DestroyThreadViewSet(read_view_set);
      DestroyThreadViewSet(write_view_set);
      DestroyImage(paint_image);
      return (Image *) NULL;
    }

  (void) SetImageType(paint_image,TrueColorType);

  /*
    Paint each row of the image.
  */
#pragma omp parallel for schedule(static,64)
  for (y=0; y < (long) image->rows; y++)
    {
      ViewInfo
        *read_view,
        *write_view;

      const PixelPacket
        *p,
        *r;
    
      PixelPacket
        *q;
    
      long
        x;

      const PixelPacket
        *s;

      MagickBool
        thread_status;
      
      thread_status=status;
      if (thread_status == MagickFail)
        continue;

      read_view=AccessThreadView(read_view_set);
      p=AcquireCacheView(read_view,-width/2,y-width/2,image->columns+width,width,
                         exception);
      write_view=AccessThreadView(write_view_set);
      q=SetCacheView(write_view,0,y,paint_image->columns,1);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        thread_status=MagickFail;
      if (thread_status != MagickFail)
        {
          for (x=(long) image->columns; x > 0; x--)
            {
              long
                v;

              unsigned long
                count;

              unsigned int
                histogram[PaintHistSize];

              /*
                Determine most frequent color.
              */
              count=0;
              (void) memset(histogram,0,sizeof(histogram));
              r=p++;
              s=r;
              for (v=width; v > 0; v--)
                {
                  register long
                    u;

                  register const PixelPacket
                    *ru;

                  ru=r;
                  for (u=width; u > 0; u--)
                    {
                      register unsigned int
                        *hp;

                      hp=histogram+ScaleQuantumToChar(PixelIntensityToQuantum(ru));
                      (*hp)++;
                      if (*hp > count)
                        {
                          s=ru;
                          count=*hp;
                        }
                      ru++;
                    }
                  r+=image->columns+width;
                }
              *q++=(*s);
            }
          if (!SyncCacheView(write_view))
            {
              thread_status=MagickFail;
              CopyException(exception,&paint_image->exception);
            }

        }
#pragma omp critical
      {
        row_count++;
        if (QuantumTick(row_count,image->rows))
          if (!MagickMonitor(OilPaintImageTag,row_count,image->rows,exception))
            thread_status=MagickFail;
          
        if (thread_status == MagickFail)
          status=MagickFail;
      }
    }

  DestroyThreadViewSet(write_view_set);
  DestroyThreadViewSet(read_view_set);
  
  paint_image->is_grayscale=image->is_grayscale;
  return(paint_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     S o l a r i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SolarizeImage() applies a special effect to the image, similar to the effect
%  achieved in a photo darkroom by selectively exposing areas of photo
%  sensitive paper to light.  Threshold ranges from 0 to MaxRGB and is a
%  measure of the extent of the solarization. False is returned if an error
%  is encountered.
%
%  The format of the SolarizeImage method is:
%
%      unsigned int SolarizeImage(Image *image,const double threshold)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o threshold:  Define the extent of the solarization.
%
%
*/
static MagickPassFail
SolarizeImagePixelsCB(void *mutable_data,         /* User provided mutable data */
                      const void *immutable_data, /* User provided immutable data */
                      Image *image,               /* Modify image */
                      PixelPacket *pixels,        /* Pixel row */
                      IndexPacket *indexes,       /* Pixel row indexes */
                      const long npixels,         /* Number of pixels in row */
                      ExceptionInfo *exception)   /* Exception report */
{

  const Quantum
    threshold = *((const double *) immutable_data);

  register long
    i;  

  ARG_NOT_USED(mutable_data);
  ARG_NOT_USED(image);
  ARG_NOT_USED(indexes);
  ARG_NOT_USED(exception);

  for (i=0; i < npixels; i++)
    {
      pixels[i].red=(pixels[i].red > threshold ?
                     MaxRGB-pixels[i].red : pixels[i].red);
      pixels[i].green=(pixels[i].green > threshold ?
                       MaxRGB-pixels[i].green : pixels[i].green);
      pixels[i].blue=(pixels[i].blue > threshold ?
                      MaxRGB-pixels[i].blue : pixels[i].blue);
    }

  return MagickPass;
}
MagickExport MagickPassFail SolarizeImage(Image *image,const double threshold)
{
#define SolarizeImageText  "  Solarize the image colors...  "

  unsigned int
    is_grayscale;

  MagickPassFail
    status=MagickPass;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  is_grayscale=image->is_grayscale;
  switch (image->storage_class)
  {
    case DirectClass:
    default:
    {
      /*
        Solarize DirectClass packets.
      */
      status=PixelIterateMonoModify(SolarizeImagePixelsCB,
                                    NULL,
                                    SolarizeImageText,
                                    NULL,&threshold,0,0,image->columns,image->rows,
                                    image,&image->exception);
      break;
    }
    case PseudoClass:
    {
      /*
        Solarize PseudoClass packets.
      */
      SolarizeImagePixelsCB(0,
                            &threshold,
                            image,
                            image->colormap,
                            (IndexPacket *) NULL,
                            image->colors,
                            &image->exception);
      status &= SyncImage(image);
      break;
    }
  }
  image->is_grayscale=is_grayscale;
  return (status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t e g a n o I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Use SteganoImage() to hide a digital watermark within the image.
%  Recover the hidden watermark later to prove that the authenticity of
%  an image.  Offset defines the start position within the image to hide
%  the watermark.
%
%  The format of the SteganoImage method is:
%
%      Image *SteganoImage(const Image *image,Image *watermark,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o watermark: The watermark image.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
#define GetBit(a,i) (((a) >> (i)) & 0x01)
#define SetBit(a,i,set) \
  a=(Quantum) ((set) ? (a) | (1UL << (i)) : (a) & ~(1UL << (i)))
#define SteganoImageText  "  Hide image...  "
MagickExport Image *SteganoImage(const Image *image,const Image *watermark,
  ExceptionInfo *exception)
{

  Image
    *stegano_image;

  long
    c,
    i,
    j,
    k,
    y;

  PixelPacket
    pixel;

  register long
    x;

  unsigned int
    is_grayscale;

  register PixelPacket
    *q;

  /*
    Initialize steganographic image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(watermark != (const Image *) NULL);
  assert(watermark->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  is_grayscale=(image->is_grayscale && watermark->is_grayscale);
  stegano_image=CloneImage(image,0,0,True,exception);
  if (stegano_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageType(stegano_image,TrueColorType);
  stegano_image->depth=QuantumDepth;
  /*
    Hide watermark in low-order bits of image.
  */
  c=0;
  i=0;
  j=0;
  k=image->offset;
  for (i=QuantumDepth-1; (i >= 0) && (j < QuantumDepth); i--)
  {
    for (y=0; (y < (long) watermark->rows) && (j < QuantumDepth); y++)
    {
      for (x=0; (x < (long) watermark->columns) && (j < QuantumDepth); x++)
      {
        pixel=AcquireOnePixel(watermark,x,y,exception);
        q=GetImagePixels(stegano_image,k % (long) stegano_image->columns,
          k/(long) stegano_image->columns,1,1);
        if (q == (PixelPacket *) NULL)
          break;
        switch ((int) c)
        {
          case 0:
          {
            SetBit(q->red,j,GetBit(PixelIntensityToQuantum(&pixel),i));
            break;
          }
          case 1:
          {
            SetBit(q->green,j,GetBit(PixelIntensityToQuantum(&pixel),i));
            break;
          }
          case 2:
          {
            SetBit(q->blue,j,GetBit(PixelIntensityToQuantum(&pixel),i));
            break;
          }
        }
        (void) SyncImage(stegano_image);
        c++;
        if (c == 3)
          c=0;
        k++;
        if (k == (long) (stegano_image->columns*stegano_image->columns))
          k=0;
        if (k == image->offset)
          j++;
      }
    }
    if (QuantumTick(i,QuantumDepth))
      if (!MagickMonitor(SteganoImageText,i,QuantumDepth,exception))
        break;
  }
  if (stegano_image->storage_class == PseudoClass)
    (void) SyncImage(stegano_image);
  stegano_image->is_grayscale=is_grayscale;
  return(stegano_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t e r e o I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StereoImage() combines two images and produces a single image that is the
%  composite of a left and right image of a stereo pair.  Special red-green
%  stereo glasses are required to view this effect.
%
%  The format of the StereoImage method is:
%
%      Image *StereoImage(const Image *image,const Image *offset_image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o stereo_image: Method StereoImage returns a pointer to the stereo
%      image.  A null image is returned if there is a memory shortage.
%
%    o image: The image.
%
%    o offset_image: Another image.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
MagickExport Image *StereoImage(const Image *image,const Image *offset_image,
  ExceptionInfo *exception)
{
#define StereoImageText  "  Stereo image...  "

  Image
    *stereo_image;

  long
    y;

  register const PixelPacket
    *p,
    *q;

  register long
    x;

  register PixelPacket
    *r;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  assert(offset_image != (const Image *) NULL);
  if ((image->columns != offset_image->columns) ||
      (image->rows != offset_image->rows))
    ThrowImageException3(ImageError,UnableToCreateStereoImage,
      LeftAndRightImageSizesDiffer);
  /*
    Initialize stereo image attributes.
  */
  stereo_image=CloneImage(image,image->columns,image->rows,True,exception);
  if (stereo_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageType(stereo_image,TrueColorType);
  /*
    Copy left image to red channel and right image to blue channel.
  */
  for (y=0; y < (long) stereo_image->rows; y++)
  {
    p=AcquireImagePixels(image,0,y,image->columns,1,exception);
    q=AcquireImagePixels(offset_image,0,y,offset_image->columns,1,exception);
    r=SetImagePixels(stereo_image,0,y,stereo_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL) ||
        (r == (PixelPacket *) NULL))
      break;
    for (x=0; x < (long) stereo_image->columns; x++)
    {
      r->red=p->red;
      r->green=q->green;
      r->blue=q->blue;
      r->opacity=(p->opacity+q->opacity)/2;
      p++;
      q++;
      r++;
    }
    if (!SyncImagePixels(stereo_image))
      break;
    if (QuantumTick(y,stereo_image->rows))
      if (!MagickMonitor(StereoImageText,y,stereo_image->rows,exception))
        break;
  }
  return(stereo_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     S w i r l I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SwirlImage() swirls the pixels about the center of the image, where
%  degrees indicates the sweep of the arc through which each pixel is moved.
%  You get a more dramatic effect as the degrees move from 1 to 360.
%
%  The format of the SwirlImage method is:
%
%      Image *SwirlImage(const Image *image,double degrees,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o degrees: Define the tightness of the swirling effect.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
MagickExport Image *SwirlImage(const Image *image,double degrees,
  ExceptionInfo *exception)
{
#define SwirlImageText  "  Swirl image...  "

  double
    cosine,
    distance,
    factor,
    radius,
    sine,
    x_center,
    x_distance,
    x_scale,
    y_center,
    y_distance,
    y_scale;

  long
    y;

  Image
    *swirl_image;

  register PixelPacket
    *q;

  register long
    x;

  /*
    Initialize swirl image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  swirl_image=CloneImage(image,image->columns,image->rows,True,exception);
  if (swirl_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageType(swirl_image,swirl_image->background_color.opacity !=
    OpaqueOpacity ? TrueColorMatteType : TrueColorType);
  /*
    Compute scaling factor.
  */
  x_center=image->columns/2.0;
  y_center=image->rows/2.0;
  radius=Max(x_center,y_center);
  x_scale=1.0;
  y_scale=1.0;
  if (image->columns > image->rows)
    y_scale=(double) image->columns/image->rows;
  else
    if (image->columns < image->rows)
      x_scale=(double) image->rows/image->columns;
  degrees=DegreesToRadians(degrees);
  /*
    Swirl each row.
  */
  for (y=0; y < (long) image->rows; y++)
  {
    q=SetImagePixels(swirl_image,0,y,swirl_image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    y_distance=y_scale*(y-y_center);
    for (x=0; x < (long) image->columns; x++)
    {
      /*
        Determine if the pixel is within an ellipse.
      */
      x_distance=x_scale*(x-x_center);
      distance=x_distance*x_distance+y_distance*y_distance;
      if (distance >= (radius*radius))
        *q=AcquireOnePixel(image,x,y,exception);
      else
        {
          /*
            Swirl the pixel.
          */
          factor=1.0-sqrt(distance)/radius;
          sine=sin(degrees*factor*factor);
          cosine=cos(degrees*factor*factor);
          *q=InterpolateColor(image,
            (cosine*x_distance-sine*y_distance)/x_scale+x_center,
            (sine*x_distance+cosine*y_distance)/y_scale+y_center,exception);
        }
      q++;
    }
    if (!SyncImagePixels(swirl_image))
      break;
    if (QuantumTick(y,image->rows))
      if (!MagickMonitor(SwirlImageText,y,image->rows,exception))
        break;
  }
  swirl_image->is_grayscale=image->is_grayscale;
  return(swirl_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     W a v e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The WaveImage() filter creates a "ripple" effect in the image by shifting
%  the pixels vertically along a sine wave whose amplitude and wavelength
%  is specified by the given parameters.
%
%  The format of the WaveImage method is:
%
%      Image *WaveImage(const Image *image,const double amplitude,
%        const double wave_length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o amplitude, frequency:  Define the amplitude and wave_length of the
%      sine wave.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
MagickExport Image *WaveImage(const Image *image,const double amplitude,
                              const double wave_length,ExceptionInfo *exception)
{
#define WaveImageText  "  Wave image...  "

  double
    *sine_map;

  Image
    *wave_image;

  long
    y;

  register long
    x;

  register PixelPacket
    *q;

  VirtualPixelMethod
    virtual_pixel_method;

  /*
    Initialize wave image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  wave_image=CloneImage(image,image->columns,(long)
    (image->rows+2.0*fabs(amplitude)),False,exception);
  if (wave_image == (Image *) NULL)
    return((Image *) NULL);

  wave_image->storage_class=DirectClass;

  /*
    If background color is non-opaque, then initialize matte channel.
  */
  if ((wave_image->background_color.opacity != OpaqueOpacity) &&
      (!wave_image->matte))
    SetImageOpacity(wave_image,OpaqueOpacity);

  /*
    Allocate sine map.
  */
  sine_map=MagickAllocateMemory(double *,wave_image->columns*sizeof(double));
  if (sine_map == (double *) NULL)
    {
      DestroyImage(wave_image);
      ThrowImageException(ResourceLimitError,MemoryAllocationFailed,
        MagickMsg(OptionError,UnableToWaveImage))
    }
  for (x=0; x < (long) wave_image->columns; x++)
    sine_map[x]=fabs(amplitude)+amplitude*sin((2*MagickPI*x)/wave_length);
  /*
    Wave image.
  */
  virtual_pixel_method=GetImageVirtualPixelMethod(image);
  if (virtual_pixel_method == UndefinedVirtualPixelMethod)
    (void) SetImageVirtualPixelMethod(image,ConstantVirtualPixelMethod);
  for (y=0; y < (long) wave_image->rows; y++)
  {
    q=SetImagePixels(wave_image,0,y,wave_image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (long) wave_image->columns; x++)
    {
      *q=InterpolateColor(image,(double) x,(double) y-sine_map[x],exception);
      q++;
    }
    if (!SyncImagePixels(wave_image))
      break;
    if (QuantumTick(y,wave_image->rows))
      if (!MagickMonitor(WaveImageText,y,wave_image->rows,exception))
        break;
  }
  (void) SetImageVirtualPixelMethod(image,virtual_pixel_method);
  MagickFreeMemory(sine_map);
  wave_image->is_grayscale=(image->is_grayscale && IsGray(wave_image->background_color));
  return(wave_image);
}
