/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           GGGG   EEEEE   OOO   M   M  EEEEE  TTTTT  RRRR   Y   Y            %
%           G      E      O   O  MM MM  E        T    R   R   Y Y             %
%           G  GG  EEE    O   O  M M M  EEE      T    RRRR     Y              %
%           G   G  E      O   O  M   M  E        T    R R      Y              %
%            GGGG  EEEEE   OOO   M   M  EEEEE    T    R  R     Y              %
%                                                                             %
%                                                                             %
%                       ImageMagick Geometry Methods                          %
%                                                                             %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                              January 2003                                   %
%                                                                             %
%                                                                             %
%  Copyright (C) 2003 ImageMagick Studio, a non-profit organization dedicated %
%  to making software imaging solutions freely available.                     %
%                                                                             %
%  Permission is hereby granted, free of charge, to any person obtaining a    %
%  copy of this software and associated documentation files ("ImageMagick"),  %
%  to deal in ImageMagick without restriction, including without limitation   %
%  the rights to use, copy, modify, merge, publish, distribute, sublicense,   %
%  and/or sell copies of ImageMagick, and to permit persons to whom the       %
%  ImageMagick is furnished to do so, subject to the following conditions:    %
%                                                                             %
%  The above copyright notice and this permission notice shall be included in %
%  all copies or substantial portions of ImageMagick.                         %
%                                                                             %
%  The software is provided "as is", without warranty of any kind, express or %
%  implied, including but not limited to the warranties of merchantability,   %
%  fitness for a particular purpose and noninfringement.  In no event shall   %
%  ImageMagick Studio be liable for any claim, damages or other liability,    %
%  whether in an action of contract, tort or otherwise, arising from, out of  %
%  or in connection with ImageMagick or the use or other dealings in          %
%  ImageMagick.                                                               %
%                                                                             %
%  Except as contained in this notice, the name of the ImageMagick Studio     %
%  shall not be used in advertising or otherwise to promote the sale, use or  %
%  other dealings in ImageMagick without prior written authorization from the %
%  ImageMagick Studio.                                                        %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/constitute.h"
#include "magick/error.h"
#include "magick/utility.h"
#include "wand/magick_compat.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m p o r t I m a g e P i x e l s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImportImagePixels() accepts pixel data and stores in the image at the
%  location you specify.  The method returns True on success otherwise False
%  if an error is encountered.  The pixel data can be either char, short int,
%  int, long, float, or double in the order specified by map.
%
%  Suppose your want want to upload the first scanline of a 640x480 image from
%  character data in red-green-blue order:
%
%      ImportImagePixels(image,0,0,640,1,"RGB",CharPixel,pixels);
%
%  The format of the ImportImagePixels method is:
%
%      unsigned int ImportImagePixels(Image *image,const long x_offset,
%        const long y_offset,const unsigned long columns,
%        const unsigned long rows,const char *map,const StorageType type,
%        const void *pixels)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o x_offset, y_offset, columns, rows:  These values define the perimeter
%      of a region of pixels you want to define.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha, C = cyan, Y = yellow, M = magenta, K = black, or
%      I = intensity (for grayscale).
%
%    o type: Define the data type of the pixels.  Float and double types are
%      normalized to [0..1] otherwise [0..MaxRGB].  Choose from these types:
%      CharPixel, ShortPixel, IntegerPixel, LongPixel, FloatPixel, or
%      DoublePixel.
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
%
*/
MagickExport unsigned int ImportImagePixels(Image *image,const long x_offset,
  const long y_offset,const unsigned long columns,const unsigned long rows,
  const char *map,const StorageType type,const void *pixels)
{
  Image
    *constitute_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);

  constitute_image=
    ConstituteImage(columns,rows,map,type,pixels,&image->exception);
  if (constitute_image)
    {
      CompositeImage(image,CopyCompositeOp,constitute_image,x_offset,
                     y_offset);
      DestroyImage(constitute_image);
      return (image->exception.severity == UndefinedException);
    }
  return (False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e A b s o l u t e G e o m e t r y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseAbsoluteGeometry() returns a region as defined by the geometry string.
%
%  The format of the ParseAbsoluteGeometry method is:
%
%      unsigned int ParseAbsoluteGeometry(const char *geometry,
%        RectangeInfo *region_info)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry (e.g. 100x100+10+10).
%
%    o region_info: The region as defined by the geometry string.
%
%
*/
MagickExport unsigned int ParseAbsoluteGeometry(const char *geometry,
  RectangleInfo *region_info)
{
  unsigned int
    flags;

  flags=GetGeometry(geometry,&region_info->x,&region_info->y,
    &region_info->width,&region_info->height);
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e G e o m e t r y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseGeometry() parses a geometry specification and returns the sigma,
%  rho, xi, and psi values.  It also returns flags that indicates which
%  of the four values (sigma, rho, xi, psi) were located in the string, and
%  whether the xi or pi values are negative.  In addition, there are flags to
%  report any meta characters (%, !, <, or >).
%
%  The format of the ParseGeometry method is:
%
%      unsigned int ParseGeometry(const char *geometry,
%        GeometryInfo *geometry_info)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry.
%
%    o geometry_info:  returns the parsed width/height/x/y in this structure.
%
%
*/
MagickExport unsigned int ParseGeometry(const char *geometry,
  GeometryInfo *geometry_info)
{
  char
    *p,
    pedantic_geometry[MaxTextExtent],
    *q;

  unsigned int
    flags;

  /*
    Remove whitespaces meta characters from geometry specification.
  */
  assert(geometry_info != (GeometryInfo *) NULL);
  flags=NoValue;
  if ((geometry == (char *) NULL) || (*geometry == '\0'))
    return(flags);
  if (strlen(geometry) >= MaxTextExtent)
    return(flags);
  (void) strncpy(pedantic_geometry,geometry,MaxTextExtent-1);
  for (p=pedantic_geometry; *p != '\0'; )
  {
    if (isspace((int) (*p)))
      {
        (void) strcpy(p,p+1);
        continue;
      }
    switch (*p)
    {
      case '%':
      {
        flags|=PercentValue;
        (void) strcpy(p,p+1);
        break;
      }
      case '!':
      {
        flags|=AspectValue;
        (void) strcpy(p,p+1);
        break;
      }
      case '<':
      {
        flags|=LessValue;
        (void) strcpy(p,p+1);
        break;
      }
      case '>':
      {
        flags|=GreaterValue;
        (void) strcpy(p,p+1);
        break;
      }
      case '@':
      {
        flags|=AreaValue;
        (void) strcpy(p,p+1);
        break;
      }
      case '-':
      case '.':
      case '+':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case 'x':
      case 'X':
      case ',':
      case '/':
      {
        p++;
        break;
      }
      default:
      {
        ExceptionInfo
          exception;

        Image
          *image;

        ImageInfo
          *image_info;

        /*
          See if we can grab geometry from an image.
        */
        GetExceptionInfo(&exception);
        image_info=CloneImageInfo((ImageInfo *) NULL);
        (void) strncpy(image_info->filename,geometry,MaxTextExtent-1);
        image=PingImage(image_info,&exception);
        if (image != (Image *) NULL)
          {
            geometry_info->rho=image->columns;
            geometry_info->sigma=image->rows;
            flags|=RhoValue | SigmaValue;
            DestroyImage(image);
          }
        DestroyImageInfo(image_info);
        DestroyExceptionInfo(&exception);
        return(flags);
      }
    }
  }
  /*
    Parse rho, sigma, xi, and psi.
  */
  p=pedantic_geometry;
  if (*p == '\0')
    return(flags);
  q=p;
  (void) strtod(p,&q);
  if ((*q == 'x') || (*q == 'X') || (*q == '/') || (*q == ',') || (*q =='\0'))
    {
      /*
        Parse rho.
      */
      q=p;
      if (LocaleNCompare(p,"0x",2) == 0)
        geometry_info->rho=strtol(p,&p,10);
      else
        geometry_info->rho=strtod(p,&p);
      if (p != q)
        flags|=RhoValue;
    }
  if ((*p == 'x') || (*p == 'X') || (*p == '/') || (*p == ','))
    {
      /*
        Parse sigma.
      */
      p++;
      q=p;
      geometry_info->sigma=strtod(p,&p);
      if (p != q)
        flags|=SigmaValue;
    }
  if ((*p == '+') || (*p == '-') || (*p == ',') || (*p == '/'))
    {
      /*
        Parse xi value.
      */
      if ((*p == ',') || (*p == '/'))
        p++;
      q=p;
      geometry_info->xi=strtod(p,&p);
      if (p != q)
        {
          flags|=XiValue;
          if (*q == '-')
            flags|=XiNegative;
        }
      if ((*p == '+') || (*p == '-') || (*p == ',') || (*p == '/'))
        {
          /*
            Parse psi value.
          */
          if ((*p == ',') || (*p == '/'))
            p++;
          q=p;
          geometry_info->psi=strtod(p,&p);
          if (p != q)
            {
              flags|=PsiValue;
              if (*q == '-')
                flags|=PsiNegative;
            }
        }
    }
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s i z e M a g i c k M e m o r y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResizeMagickMemory() changes the size of the memory and returns a pointer to
%  the (possibly moved) block.  The contents will be unchanged up to the
%  lesser of the new and old sizes.
%
%  The format of the ResizeMagickMemory method is:
%
%      void *ResizeMagickMemory(void *memory,const size_t size)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a memory allocation.  On return the pointer
%      may change but the contents of the original allocation will not.
%
%    o size: The new size of the allocated memory.
%
%
*/
MagickExport void *ResizeMagickMemory(void *memory,const size_t size)
{
  void
    *allocation;

  if (memory == (void *) NULL)
    return(AcquireMagickMemory(size));
  allocation=realloc(memory,size);
  if (allocation == (void *) NULL)
    RelinquishMagickMemory(memory);
  return(allocation);
}
