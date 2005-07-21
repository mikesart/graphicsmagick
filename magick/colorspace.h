/*
  Copyright (C) 2003 GraphicsMagick Group
  Copyright (C) 2002 ImageMagick Studio
  Copyright 1991-1999 E. I. du Pont de Nemours and Company
 
  This program is covered by multiple licenses, which are described in
  Copyright.txt. You should have received a copy of Copyright.txt with this
  package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
 
  GraphicsMagick Colorspace Methods. 
*/
#ifndef _MAGICK_COLORSPACE_H
#define _MAGICK_COLORSPACE_H
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


#if (QuantumDepth == 8) || (QuantumDepth == 16)
  /*
    intensity=0.299*red+0.587*green+0.114*blue.
    Premultiply by 1024 to obtain integral values, and then divide
    result by 1024 by shifting to the right by 10 bits.
  */
#define PixelIntensityRec601(pixel) \
  ((unsigned int) \
   (((unsigned int) (pixel)->red*306U+ \
     (unsigned int) (pixel)->green*601U+ \
     (unsigned int) (pixel)->blue*117U) \
    >> 10U))
#elif (QuantumDepth == 32)
  /*
    intensity=0.299*red+0.587*green+0.114*blue.
  */
#define PixelIntensityRec601(pixel) \
  ((unsigned int) \
   (((double)306.0*(pixel)->red+ \
     (double)601.0*(pixel)->green+ \
     (double)117.0*(pixel)->blue) \
    / 1024.0))
#endif

  /*
    intensity=0.2126*red+0.7152*green+0.0722*blue
  */
#define PixelIntensityRec709(pixel) \
  ((unsigned int) \
   (0.2126*(pixel)->red+ \
    0.7152*(pixel)->green+ \
    0.0722*(pixel)->blue))

#define PixelIntensity(pixel) PixelIntensityRec601(pixel)
#define PixelIntensityToDouble(pixel) ((double)PixelIntensity(pixel))
#define PixelIntensityToQuantum(pixel) ((Quantum)PixelIntensity(pixel))
#define IsGrayColorspace(colorspace) \
  ( \
    (colorspace == Rec601LumaColorspace) || \
    (colorspace == Rec709LumaColorspace) \
  )
#define IsRGBColorspace(colorspace) \
  ( \
    (IsGrayColorspace(colorspace)) || \
    (colorspace == RGBColorspace) || \
    (colorspace == TransparentColorspace) \
  )
#define IsYCbCrColorspace(colorspace) \
  ( \
    (colorspace == Rec601YCbCrColorspace) || \
    (colorspace == Rec709YCbCrColorspace) \
  )

#define GRAYColorspace Rec601LumaColorspace
#define YCbCrColorspace Rec601YCbCrColorspace
typedef enum
{
  UndefinedColorspace,
  RGBColorspace,         /* Plain old RGB colorspace */
  Rec601LumaColorspace,  /* Luma according to ITU-R 601 (grayscale) */
  TransparentColorspace, /* RGB but preserve matte channel during quantize */
  OHTAColorspace,
  XYZColorspace,         /* CIE XYZ */
  Rec601YCbCrColorspace, /* YCbCr according to ITU-R 601 */
  YCCColorspace,         /* Kodak PhotoCD PhotoYCC */
  YIQColorspace,
  YPbPrColorspace,
  YUVColorspace,
  CMYKColorspace,        /* Cyan, magenta, yellow, black, alpha */
  sRGBColorspace,        /* Kodak PhotoCD sRGB */
  HSLColorspace,         /* Hue, saturation, luminosity */
  HWBColorspace,         /* Hue, whiteness, blackness */
  LABColorspace,         /* LAB colorspace not supported yet other than via lcms */
  Rec709LumaColorspace,  /* Luma according to ITU-R 709 (grayscale) */
  CineonLogRGBColorspace,/* RGB data with Cineon Log scaling, 2.048 density range */
  Rec709YCbCrColorspace  /* YCbCr according to ITU-R 709 */
} ColorspaceType;

extern MagickExport MagickPassFail
  RGBTransformImage(ImagePtr,const ColorspaceType),
  TransformColorspace(ImagePtr,const ColorspaceType),
  TransformRGBImage(ImagePtr,const ColorspaceType);

extern MagickExport ColorspaceType
  StringToColorspaceType(const char *colorspace_string);

extern MagickExport const char
  *ColorspaceTypeToString(const ColorspaceType colorspace);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _MAGICK_COLORSPACE_H */
