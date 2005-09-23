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
%                        FFFFF  IIIII  TTTTT  SSSSS                           %
%                        F        I      T    SS                              %
%                        FFF      I      T     SSS                            %
%                        F        I      T       SS                           %
%                        F      IIIII    T    SSSSS                           %
%                                                                             %
%                                                                             %
%            Read/Write Flexible Image Transport System Images.               %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/pixel_cache.h"
#include "magick/color.h"
#include "magick/constitute.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static unsigned int
  WriteFITSImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s F I T S                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsFITS returns True if the image format type, identified by the
%  magick string, is FITS.
%
%  The format of the IsFITS method is:
%
%      unsigned int IsFITS(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsFITS returns True if the image format type is FITS.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static unsigned int IsFITS(const unsigned char *magick,const size_t length)
{
  if (length < 6)
    return(False);
  if (LocaleNCompare((char *) magick,"IT0",3) == 0)
    return(True);
  if (LocaleNCompare((char *) magick,"SIMPLE",6) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F I T S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadFITSImage reads a FITS image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadFITSImage method is:
%
%      Image *ReadFITSImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: Method ReadFITSImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o filename: Specifies the name of the image to read.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadFITSImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  typedef struct _FITSInfo
  {
    int
      simple,
      bits_per_pixel,
      columns,
      rows,
      number_axes,
      number_scenes;

    double
      min_data,
      max_data,
      zero,
      scale;
  } FITSInfo;

  char
    keyword[MaxTextExtent],
    value[MaxTextExtent];

  double
    exponential[2048],
    pixel,
    scale,
    scaled_pixel;

  FITSInfo
    fits_info;

  Image
    *image;

  IndexPacket
    index;

  int
    c,
    quantum;

  long
    exponent,
    j,
    k,
    l,
    packet_size,
    scene,
    y;

  register IndexPacket
    *indexes;

  register long
    x;

  register PixelPacket
    *q;

  register long
    i;

  register unsigned char
    *p;

  size_t
    count;

  unsigned char
    *fits_pixels,
    long_quantum[8];

  unsigned int
    status,
    value_expected;

  unsigned long
    number_pixels;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AllocateImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == False)
    ThrowReaderException(FileOpenError,UnableToOpenFile,image);
  /*
    Initialize image header.
  */
  fits_info.simple=False;
  fits_info.bits_per_pixel=8;
  fits_info.columns=1;
  fits_info.rows=1;
  fits_info.number_axes=0;
  fits_info.number_scenes=1;
  fits_info.min_data=0.0;
  fits_info.max_data=0.0;
  fits_info.zero=0.0;
  fits_info.scale=1.0;
  /*
    Decode image header.
  */
  c=ReadBlobByte(image);
  if (c == EOF)
    {
      DestroyImage(image);
      return((Image *) NULL);
    }
  for ( ; ; )
  {
    if (!isalnum((int) c))
      c=ReadBlobByte(image);
    else
      {
        register char
          *p;

        /*
          Determine a keyword and its value.
        */
        p=keyword;
        do
        {
          if ((p-keyword) < (MaxTextExtent-1))
            *p++=c;
          c=ReadBlobByte(image);
        } while (isalnum(c) || (c == '_'));
        *p='\0';
        if (LocaleCompare(keyword,"END") == 0)
          break;
        value_expected=False;
        while (isspace(c) || (c == '='))
        {
          if (c == '=')
            value_expected=True;
          c=ReadBlobByte(image);
        }
        if (value_expected == False)
          continue;
        p=value;
        while (isalnum(c) || (c == '-') || (c == '+') || (c == '.'))
        {
          if ((p-value) < (MaxTextExtent-1))
            *p++=c;
          c=ReadBlobByte(image);
        }
        *p='\0';
        /*
          Assign a value to the specified keyword.
        */
        if (LocaleCompare(keyword,"SIMPLE") == 0)
          fits_info.simple=(*value == 'T') || (*value == 't');
        if (LocaleCompare(keyword,"BITPIX") == 0)
          fits_info.bits_per_pixel=atoi(value);
        if (LocaleCompare(keyword,"NAXIS") == 0)
          fits_info.number_axes=atoi(value);
        if (LocaleCompare(keyword,"NAXIS1") == 0)
          fits_info.columns= atoi(value);
        if (LocaleCompare(keyword,"NAXIS2") == 0)
          fits_info.rows= atoi(value);
        if (LocaleCompare(keyword,"NAXIS3") == 0)
          fits_info.number_scenes=atoi(value);
        if (LocaleCompare(keyword,"DATAMAX") == 0)
          fits_info.max_data=atof(value);
        if (LocaleCompare(keyword,"DATAMIN") == 0)
          fits_info.min_data=atof(value);
        if (LocaleCompare(keyword,"BZERO") == 0)
          fits_info.zero=atof(value);
        if (LocaleCompare(keyword,"BSCALE") == 0)
          fits_info.scale=atof(value);
      }
    while ((TellBlob(image) % 80) != 0)
      c=ReadBlobByte(image);
    c=ReadBlobByte(image);
  }
  while ((TellBlob(image) % 2880) != 0)
    c=ReadBlobByte(image);
  /*
    Verify that required image information is defined.
  */
  number_pixels=fits_info.columns*fits_info.rows;
  if ((!fits_info.simple) || (fits_info.number_axes < 1) ||
      (fits_info.number_axes > 4) || (number_pixels == 0))
    ThrowReaderException(CorruptImageError,ImageTypeNotSupported,image);
  if (fits_info.bits_per_pixel == -32)
    {
      exponential[150]=1.0;
      for (i=151; i < 256; i++)
        exponential[i]=2.0*exponential[i-1];
      for (i=149; i >= 0; i--)
        exponential[i]=exponential[i+1]/2.0;
    }
  if (fits_info.bits_per_pixel == -64)
    {
      exponential[1075]=1.0;
      for (i=1076; i < 2048; i++)
        exponential[i]=2.0*exponential[i-1];
      for (i=1074; i >= 0; i--)
        exponential[i]=exponential[i+1]/2.0;
    }
  for (scene=0; scene < (long) fits_info.number_scenes; scene++)
  {
    /*
      Create linear colormap.
    */
    image->columns=fits_info.columns;
    image->rows=fits_info.rows;
    image->depth=fits_info.bits_per_pixel <= 8 ? 8 : QuantumDepth;
    image->storage_class=PseudoClass;
    image->scene=scene;
    if (!AllocateImageColormap(image,1 << image->depth))
      ThrowReaderException(FileOpenError,UnableToOpenFile,image);
    if (image_info->ping && (image_info->subrange != 0))
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    /*
      Initialize image structure.
    */
    packet_size=fits_info.bits_per_pixel/8;
    if (packet_size < 0)
      packet_size=(-packet_size);
    number_pixels=image->columns*image->rows;
    fits_pixels=MagickAllocateMemory(unsigned char *,packet_size*number_pixels);
    if (fits_pixels == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
    /*
      Convert FITS pixels to pixel packets.
    */
    count=ReadBlob(image,packet_size*number_pixels,fits_pixels);
    if (count == 0)
      ThrowReaderException(CorruptImageError,InsufficientImageDataInFile,
        image);
    if ((fits_info.min_data == 0.0) && (fits_info.max_data == 0.0))
      {
        /*
          Determine minimum and maximum intensity.
        */
        p=fits_pixels;
        long_quantum[0]=(*p);
        quantum=(*p++);
        for (j=1; j <= (packet_size-1); j++)
        {
          long_quantum[j]=(*p);
          quantum=(quantum << 8U) | (*p++);
        }
        pixel=(double) quantum;
        if (fits_info.bits_per_pixel == 16)
          if (pixel > 32767)
            pixel-=65536L;
        if (fits_info.bits_per_pixel == -32)
          {
            j=((long) long_quantum[1] << 16) | ((long) long_quantum[2] << 8) |
               (long) long_quantum[3];
            k=(int) *long_quantum;
            exponent=((k & 127) << 1) | (j >> 23);
            *(float *) long_quantum=
              exponential[exponent]*(float) (j | 0x800000);
            if ((exponent | j) == 0)
              *(float *) long_quantum=0.0;
            if (k & 128)
              *(float *) long_quantum=(-(*(float *) long_quantum));
            pixel=(double) (*((float *) long_quantum));
          }
        if (fits_info.bits_per_pixel == -64)
          {
            j=((long) long_quantum[1] << 24) |
              ((long) long_quantum[2] << 16) |
              ((long) long_quantum[3] << 8) |
               (long) long_quantum[4];
            k=(int) *long_quantum;
            l=((int) long_quantum[5] << 16) | ((int) long_quantum[6] << 8) |
               (int) long_quantum[7];
            exponent=((k & 127) << 4) | (j >> 28);
            *(double *) long_quantum=exponential[exponent]*(16777216.0*
              (double) ((j & 0x0FFFFFFF) | 0x10000000)+(double) l);
            if ((exponent | j | l) == 0)
              *(double *) long_quantum=0.0;
            if (k & 128)
              *(double *)long_quantum=(-(*(double *) long_quantum));
            pixel=(double) (*((double *) long_quantum));
          }
        fits_info.min_data=pixel*fits_info.scale-fits_info.zero;
        fits_info.max_data=pixel*fits_info.scale-fits_info.zero;
        for (i=1; i < (long) number_pixels; i++)
        {
          long_quantum[0]=(*p);
          quantum=(*p++);
          for (j=1; j <= (packet_size-1); j++)
          {
            long_quantum[j]=(*p);
            quantum=(quantum << 8) | (*p++);
          }
          pixel=(double) quantum;
          if (fits_info.bits_per_pixel == 16)
            if (pixel > 32767)
              pixel-=65536L;
          if (fits_info.bits_per_pixel == -32)
            {
              j=((long) long_quantum[1] << 16) | ((long) long_quantum[2] << 8) |
                 (long) long_quantum[3];
              k=(int) *long_quantum;
              exponent=((k & 127) << 1) | (j >> 23);
              *(float *) long_quantum=
                exponential[exponent]*(float) (j | 0x800000);
              if ((exponent | j) == 0)
                *(float *) long_quantum=0.0;
              if (k & 128)
                *(float *) long_quantum=(-(*(float *) long_quantum));
              pixel=(double) (*((float *) long_quantum));
            }
          if (fits_info.bits_per_pixel == -64)
            {
              j=((long) long_quantum[1] << 24) |
                ((long) long_quantum[2] << 16) |
                ((long) long_quantum[3] << 8) |
                 (long) long_quantum[4];
              k=(int) *long_quantum;
              l=((int) long_quantum[5] << 16) | ((int) long_quantum[6] << 8) |
                 (int) long_quantum[7];
              exponent=((k & 127) << 4) | (j >> 28);
              *(double *) long_quantum=exponential[exponent]*(16777216.0*
                (double) ((j & 0x0FFFFFFF) | 0x10000000)+(double) l);
              if ((exponent | j | l) == 0)
                *(double *) long_quantum=0.0;
              if (k & 128)
                *(double *)long_quantum=(-(*(double *) long_quantum));
              pixel=(double) (*((double *) long_quantum));
            }
          scaled_pixel=pixel*fits_info.scale-fits_info.zero;
          if (scaled_pixel < fits_info.min_data)
            fits_info.min_data=scaled_pixel;
          if (scaled_pixel > fits_info.max_data)
            fits_info.max_data=scaled_pixel;
        }
      }
    /*
      Convert FITS pixels to pixel packets.
    */
    scale=1.0;
    if ((fits_info.bits_per_pixel < 0) ||
        ((fits_info.max_data-fits_info.min_data) > ((1 << image->depth)-1)))
      scale=((1 << image->depth)-1)/(fits_info.max_data-fits_info.min_data);
    p=fits_pixels;
    for (y=(long) image->rows-1; y >= 0; y--)
    {
      q=SetImagePixels(image,0,y,image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      indexes=GetIndexes(image);
      for (x=0; x < (long) image->columns; x++)
      {
        long_quantum[0]=(*p);
        quantum=(*p++);
        for (j=1; j <= (packet_size-1); j++)
        {
          long_quantum[j]=(*p);
          quantum=(quantum << 8) | (*p++);
        }
        pixel=(double) quantum;
        if (fits_info.bits_per_pixel == 16)
          if (pixel > 32767)
            pixel-=65536L;
        if (fits_info.bits_per_pixel == -32)
          {
            j=((long) long_quantum[1] << 16) | ((long) long_quantum[2] << 8) |
               (long) long_quantum[3];
            k=(int) *long_quantum;
            exponent=((k & 127) << 1) | (j >> 23);
            *(float *) long_quantum=
              exponential[exponent]*(float) (j | 0x800000);
            if ((exponent | j) == 0)
              *(float *) long_quantum=0.0;
            if (k & 128)
              *(float *) long_quantum=(-(*(float *) long_quantum));
            pixel=(double) (*((float *) long_quantum));
          }
        if (fits_info.bits_per_pixel == -64)
          {
            j=((long) long_quantum[1] << 24) |
              ((long) long_quantum[2] << 16) |
              ((long) long_quantum[3] << 8) |
               (long) long_quantum[4];
            k=(int) *long_quantum;
            l=((long) long_quantum[5] << 16) | ((long) long_quantum[6] << 8) |
               (long) long_quantum[7];
            exponent=((k & 127) << 4) | (j >> 28);
            *(double *) long_quantum=exponential[exponent]*(16777216.0*
              (double) ((j & 0x0FFFFFFF) | 0x10000000)+(double) l);
            if ((exponent | j | l) == 0)
              *(double *) long_quantum=0.0;
            if (k & 128)
              *(double *)long_quantum=(-(*(double *) long_quantum));
            pixel=(double) (*((double *) long_quantum));
          }
        scaled_pixel=scale*
          (pixel*fits_info.scale-fits_info.min_data-fits_info.zero);
        index=(IndexPacket) ((scaled_pixel < 0) ? 0 :
          (scaled_pixel > ((1 << image->depth)-1)) ? ((1 << image->depth)-1) :
          scaled_pixel+0.5);
        index=(IndexPacket) (index);
        VerifyColormapIndex(image,index);
        indexes[x]=index;
        *q++=image->colormap[index];
      }
      if (!SyncImagePixels(image))
        break;
      if (QuantumTick(y,image->rows))
        if (!MagickMonitor(LoadImageText,y,image->rows,exception))
          break;
    }
    MagickFreeMemory(fits_pixels);
    if (EOFBlob(image))
      {
        ThrowException(exception,CorruptImageError,UnexpectedEndOfFile,
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    if (scene < (long) (fits_info.number_scenes-1))
      {
        /*
          Allocate next image structure.
        */
        AllocateNextImage(image_info,image);
        if (image->next == (Image *) NULL)
          {
            DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        if (!MagickMonitor(LoadImagesText,TellBlob(image),GetBlobSize(image),exception))
          break;
      }
  }
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r F I T S I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterFITSImage adds attributes for the FITS image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterFITSImage method is:
%
%      RegisterFITSImage(void)
%
*/
ModuleExport void RegisterFITSImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("FITS");
  entry->decoder=(DecoderHandler) ReadFITSImage;
  entry->encoder=(EncoderHandler) WriteFITSImage;
  entry->magick=(MagickHandler) IsFITS;
  entry->adjoin=False;
  entry->description=AcquireString("Flexible Image Transport System");
  entry->module=AcquireString("FITS");
  (void) RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r F I T S I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterFITSImage removes format registrations made by the
%  FITS module from the list of supported formats.
%
%  The format of the UnregisterFITSImage method is:
%
%      UnregisterFITSImage(void)
%
*/
ModuleExport void UnregisterFITSImage(void)
{
  (void) UnregisterMagickInfo("FITS");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F I T S I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteFITSImage writes a Flexible Image Transport System image to a
%  file as gray scale intensities [0..255].
%
%  The format of the WriteFITSImage method is:
%
%      unsigned int WriteFITSImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteFITSImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o image:  A pointer to an Image structure.
%
%
*/
static unsigned int WriteFITSImage(const ImageInfo *image_info,Image *image)
{
  char
    buffer[81],
    *fits_info;

  long
    y;

  register const PixelPacket
    *p;

  register long
    i;

  unsigned char
    *pixels;

  unsigned int
    depth,
    quantum_size,
    status;

  unsigned long
    packet_size;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == False)
    ThrowWriterException(FileOpenError,UnableToOpenFile,image);
  (void) TransformColorspace(image,RGBColorspace);
  if (image->depth > 8)
    depth=16;
  else
    depth=8;
  /*
    Allocate image memory.
  */
  if (depth <= 8)
    quantum_size=8;
  else
    quantum_size=16;

  packet_size=quantum_size/8;
  fits_info=MagickAllocateMemory(char *,2880);
  pixels=MagickAllocateMemory(unsigned char *,packet_size*image->columns);
  if ((fits_info == (char *) NULL) || (pixels == (unsigned char *) NULL))
    ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,image);
  /*
    Initialize image header.
  */
  for (i=0; i < 2880; i++)
    fits_info[i]=' ';
  (void) strcpy(buffer,"SIMPLE  =                    T");
  (void) strncpy(fits_info+0,buffer,strlen(buffer));
  FormatString(buffer,"BITPIX  =                    %u",depth);
  (void) strncpy(fits_info+80,buffer,strlen(buffer));
  (void) strcpy(buffer,"NAXIS   =                    2");
  (void) strncpy(fits_info+160,buffer,strlen(buffer));
  FormatString(buffer,"NAXIS1  =           %10lu",image->columns);
  (void) strncpy(fits_info+240,buffer,strlen(buffer));
  FormatString(buffer,"NAXIS2  =           %10lu",image->rows);
  (void) strncpy(fits_info+320,buffer,strlen(buffer));
  FormatString(buffer,"DATAMIN =           %10u",0);
  (void) strncpy(fits_info+400,buffer,strlen(buffer));
  FormatString(buffer,"DATAMAX =           %10lu",(unsigned long) MaxValueGivenBits(depth));
  (void) strncpy(fits_info+480,buffer,strlen(buffer));
  (void) strcpy(buffer,"HISTORY Created by GraphicsMagick.");
  (void) strncpy(fits_info+560,buffer,strlen(buffer));
  (void) strcpy(buffer,"END");
  (void) strncpy(fits_info+640,buffer,strlen(buffer));
  (void) WriteBlob(image,2880,(char *) fits_info);
  MagickFreeMemory(fits_info);
  /*
    Convert image to fits scale PseudoColor class.
  */
  for (y=(long) image->rows-1; y >= 0; y--)
  {
    p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    (void) ExportImagePixelArea(image,GrayQuantum,quantum_size,pixels,0);
    (void) WriteBlob(image,packet_size*image->columns,pixels);
    if (QuantumTick(image->rows-y-1,image->rows))
      {
        status=MagickMonitor(SaveImageText,image->rows-y-1,image->rows,
          &image->exception);
        if (status == False)
          break;
      }
  }
  MagickFreeMemory(pixels);
  CloseBlob(image);
  return(True);
}
