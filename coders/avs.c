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
%                             AAA   V   V  SSSSS                              %
%                            A   A  V   V  SS                                 %
%                            AAAAA  V   V   SSS                               %
%                            A   A   V V      SS                              %
%                            A   A    V    SSSSS                              %
%                                                                             %
%                                                                             %
%                        Read/Write AVS X Image Format.                       %
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
#include "magick/cache.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static unsigned int
  WriteAVSImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d A V S I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadAVSImage reads an AVS X image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadAVSImage method is:
%
%      Image *ReadAVSImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadAVSImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadAVSImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  long
    y;

  register long
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  size_t
    count;

  unsigned char
    *pixels;

  unsigned int
    status;

  unsigned long
    height,
    width;

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
    Read AVS image.
  */
  width=ReadBlobMSBLong(image);
  height=ReadBlobMSBLong(image);
  if ((width == (unsigned long) ~0) || (height == (unsigned long) ~0))
    ThrowReaderException(CorruptImageError,ImproperImageHeader,image);
  do
  {
    /*
      Convert AVS raster image to pixel packets.
    */
    image->columns=width;
    image->rows=height;
    image->depth=8;
    if (image_info->ping && (image_info->subrange != 0))
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    pixels=MagickAllocateMemory(unsigned char *,4*image->columns);
    if (pixels == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
    for (y=0; y < (long) image->rows; y++)
    {
      count=ReadBlob(image,4*image->columns,pixels);
      if (count == 0)
        ThrowReaderException(CorruptImageError,UnableToReadImageData,image);
      p=pixels;
      q=SetImagePixels(image,0,y,image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      for (x=0; x < (long) image->columns; x++)
      {
        q->opacity=(Quantum) (MaxRGB-ScaleCharToQuantum(*p++));
        q->red=ScaleCharToQuantum(*p++);
        q->green=ScaleCharToQuantum(*p++);
        q->blue=ScaleCharToQuantum(*p++);
        image->matte|=(q->opacity != OpaqueOpacity);
        q++;
      }
      if (!SyncImagePixels(image))
        break;
      if (image->previous == (Image *) NULL)
        if (QuantumTick(y,image->rows))
          if (!MagickMonitor(LoadImageText,y,image->rows,exception))
            break;
    }
    MagickFreeMemory(pixels);
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
    width=ReadBlobMSBLong(image);
    height=ReadBlobMSBLong(image);
    if ((width != (unsigned long) ~0) && (height != (unsigned long) ~0))
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
        status=MagickMonitor(LoadImagesText,TellBlob(image),GetBlobSize(image),
          exception);
        if (status == False)
          break;
      }
  } while ((width != (unsigned long) ~0) && (height != (unsigned long) ~0));
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
%   R e g i s t e r A V S I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterAVSImage adds attributes for the AVS image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterAVSImage method is:
%
%      RegisterAVSImage(void)
%
*/
ModuleExport void RegisterAVSImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("AVS");
  entry->decoder=(DecoderHandler) ReadAVSImage;
  entry->encoder=(EncoderHandler) WriteAVSImage;
  entry->description=AcquireString("AVS X image");
  entry->module=AcquireString("AVS");
  (void) RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r A V S I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterAVSImage removes format registrations made by the
%  AVS module from the list of supported formats.
%
%  The format of the UnregisterAVSImage method is:
%
%      UnregisterAVSImage(void)
%
*/
ModuleExport void UnregisterAVSImage(void)
{
  (void) UnregisterMagickInfo("AVS");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e A V S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteAVSImage writes an image to a file in AVS X image format.
%
%  The format of the WriteAVSImage method is:
%
%      unsigned int WriteAVSImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteAVSImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o image:  A pointer to an Image structure.
%
%
*/
static unsigned int WriteAVSImage(const ImageInfo *image_info,Image *image)
{
  register const PixelPacket
    *p;

  register long
    x,
    y;

  register unsigned char
    *q;

  unsigned char
    *pixels;

  unsigned int
    status;

  unsigned long
    scene;

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
  scene=0;
  do
  {
    /*
      Write AVS header.
    */
    TransformColorspace(image,RGBColorspace);
    (void) WriteBlobMSBLong(image,image->columns);
    (void) WriteBlobMSBLong(image,image->rows);
    /*
      Allocate memory for pixels.
    */
    pixels=MagickAllocateMemory(unsigned char *,image->columns*sizeof(PixelPacket));
    if (pixels == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,image);
    /*
      Convert MIFF to AVS raster pixels.
    */
    for (y=0; y < (long) image->rows; y++)
    {
      p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
      if (p == (PixelPacket *) NULL)
        break;
      q=pixels;
      for (x=0; x < (long) image->columns; x++)
      {
        *q++=ScaleQuantumToChar(
          MaxRGB-(image->matte ? p->opacity : OpaqueOpacity));
        *q++=ScaleQuantumToChar(p->red);
        *q++=ScaleQuantumToChar(p->green);
        *q++=ScaleQuantumToChar(p->blue);
        p++;
      }
      (void) WriteBlob(image,q-pixels,(char *) pixels);
      if (image->previous == (Image *) NULL)
        if (QuantumTick(y,image->rows))
          if (!MagickMonitor(SaveImageText,y,image->rows,&image->exception))
            break;
    }
    MagickFreeMemory(pixels);
    if (image->next == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=MagickMonitor(SaveImagesText,scene++,GetImageListLength(image),
      &image->exception);
    if (status == False)
      break;
  } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  CloseBlob(image);
  return(True);
}
