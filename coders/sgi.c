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
%                            SSSSS   GGGG  IIIII                              %
%                            SS     G        I                                %
%                             SSS   G  GG    I                                %
%                               SS  G   G    I                                %
%                            SSSSS   GGG   IIIII                              %
%                                                                             %
%                                                                             %
%                     Read/Write Irix RGB Image Format.                       %
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
#include "magick/color.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/utility.h"

/*
  Typedef declaractions.
*/
typedef struct _SGIInfo
{
  unsigned short
    magic;

  unsigned char
    storage,
    bytes_per_pixel;

  unsigned short
    dimension,
    columns,
    rows,
    depth;

  unsigned long
    minimum_value,
    maximum_value;

  unsigned char
    filler[492];
} SGIInfo;

/*
  Forward declarations.
*/
static unsigned int
  WriteSGIImage(const ImageInfo *,Image *);
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s S G I                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsSGI returns True if the image format type, identified by the
%  magick string, is SGI.
%
%  The format of the IsSGI method is:
%
%      unsigned int IsSGI(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsSGI returns True if the image format type is SGI.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static unsigned int IsSGI(const unsigned char *magick,const size_t length)
{
  if (length < 2)
    return(False);
  if (memcmp(magick,"\001\332",2) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S G I I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadSGIImage reads a SGI RGB image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadSGIImage method is:
%
%      Image *ReadSGIImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadSGIImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/

static void SGIDecode(const unsigned long bytes_per_pixel,
  unsigned char *max_packets,unsigned char *pixels)
{
  long
    count;

  register unsigned char
    *p,
    *q;

  unsigned long
    pixel;

  p=max_packets;
  q=pixels;
  if (bytes_per_pixel == 2)
    {
      for ( ; ; )
      {
        pixel=(*p++) << 8U;
        pixel|=(*p++);
        count=(long) (pixel & 0x7fU);
        if (count == 0)
          break;
        if (pixel & 0x80U)
          for ( ; count != 0U; count--)
          {
            *q=(*p++);
            *(q+1)=(*p++);
            q+=8U;
          }
        else
          {
            pixel=(*p++) << 8U;
            pixel|=(*p++);
            for ( ; count != 0; count--)
            {
              *q=(unsigned char) (pixel >> 8U);
              *(q+1)=(unsigned char) pixel;
              q+=8U;
            }
          }
      }
      return;
    }
  for ( ; ; )
  {
    pixel=(*p++);
    count=(long) (pixel & 0x7f);
    if (count == 0)
      break;
    if (pixel & 0x80)
      for ( ; count != 0; count--)
      {
        *q=(*p++);
        q+=4;
      }
    else
      {
        pixel=(*p++);
        for ( ; count != 0; count--)
        {
          *q=(unsigned char) pixel;
          q+=4;
        }
      }
  }
}

static Image *ReadSGIImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  long
    y,
    z;

  register IndexPacket
    *indexes;

  register long
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  SGIInfo
    iris_info;

  unsigned char
    *iris_pixels;

  unsigned int
    status;

  unsigned long
    bytes_per_pixel,
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
    Read SGI raster header.
  */
  iris_info.magic=ReadBlobMSBShort(image);
  do
  {
    /*
      Verify SGI identifier.
    */
    if (iris_info.magic != 0x01DA)
      ThrowReaderException(CorruptImageError,ImproperImageHeader,image);
    iris_info.storage=ReadBlobByte(image);
    if (iris_info.storage == 0x01)
      image->compression=RLECompression;
    iris_info.bytes_per_pixel=ReadBlobByte(image);
    iris_info.dimension=ReadBlobMSBShort(image);
    iris_info.columns=ReadBlobMSBShort(image);
    iris_info.rows=ReadBlobMSBShort(image);
    iris_info.depth=ReadBlobMSBShort(image);
    image->columns=iris_info.columns;
    image->rows=iris_info.rows;
    image->depth=iris_info.depth <= 8 ? 8 : QuantumDepth;
    if (iris_info.depth < 3)
      {
        image->storage_class=PseudoClass;
        image->colors=256;
      }
    if (image_info->ping && (image_info->subrange != 0))
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    iris_info.minimum_value=ReadBlobMSBLong(image);
    iris_info.maximum_value=ReadBlobMSBLong(image);
    (void) ReadBlob(image,(unsigned int) sizeof(iris_info.filler),
      (char *) iris_info.filler);
    /*
      Allocate SGI pixels.
    */
    bytes_per_pixel=iris_info.bytes_per_pixel;
    number_pixels=iris_info.columns*iris_info.rows;
    iris_pixels=MagickAllocateMemory(unsigned char *,
      4*bytes_per_pixel*number_pixels);
    if (iris_pixels == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
    if (iris_info.storage != 0x01)
      {
        unsigned char
          *scanline;

        /*
          Read standard image format.
        */
        scanline=MagickAllocateMemory(unsigned char *,
          bytes_per_pixel*iris_info.columns);
        if (scanline == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,
            image);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "   Reading SGI scanlines"); 
        for (z=0; z < (int) iris_info.depth; z++)
        {
          p=iris_pixels+bytes_per_pixel*z;
          for (y=0; y < (long) iris_info.rows; y++)
          {
            (void) ReadBlob(image,bytes_per_pixel*iris_info.columns,
              (char *) scanline);
            if (EOFBlob(image))
              {
                ThrowReaderException(CorruptImageError,
                  UnexpectedEndOfFile, image);
                break;
              }
            if (bytes_per_pixel == 2)
              for (x=0; x < (long) iris_info.columns; x++)
              {
                *p=scanline[2*x];
                *(p+1)=scanline[2*x+1];
                p+=8;
              }
            else
              for (x=0; x < (long) iris_info.columns; x++)
              {
                *p=scanline[x];
                p+=4;
              }
          }
          if (EOFBlob(image))
            break;
        }
        MagickFreeMemory(scanline);
      }
    else
      {
        unsigned char
          *max_packets;

        unsigned int
          data_order;

        unsigned long
          offset,
          *offsets,
          *runlength;

        /*
          Read runlength-encoded image format.
        */
        offsets=MagickAllocateMemory(unsigned long *,iris_info.rows*
          iris_info.depth*sizeof(unsigned long));
        max_packets=MagickAllocateMemory(unsigned char *,4*iris_info.columns+10);
        runlength=MagickAllocateMemory(unsigned long *,iris_info.rows*
          iris_info.depth*sizeof(unsigned long));
        if ((offsets == (unsigned long *) NULL) ||
            (max_packets == (unsigned char *) NULL) ||
            (runlength == (unsigned long *) NULL))
          ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,
            image);
        for (i=0; i < (int) (iris_info.rows*iris_info.depth); i++)
          offsets[i]=ReadBlobMSBLong(image);
        for (i=0; i < (int) (iris_info.rows*iris_info.depth); i++)
          runlength[i]=ReadBlobMSBLong(image);
        if (EOFBlob(image))
          {
            ThrowReaderException(CorruptImageError,
              UnexpectedEndOfFile, image);
          }
        /*
          Check data order.
        */
        offset=0;
        data_order=0;
        for (y=0; ((y < (long) iris_info.rows) && !data_order); y++)
          for (z=0; ((z < (int) iris_info.depth) && !data_order); z++)
          {
            if (offsets[y+z*iris_info.rows] < offset)
              data_order=1;
            offset=offsets[y+z*iris_info.rows];
          }
        offset=512+4*bytes_per_pixel*2*(iris_info.rows*iris_info.depth);
        if (data_order == 1)
          {
            for (z=0; z < (int) iris_info.depth; z++)
            {
              p=iris_pixels;
              for (y=0; y < (long) iris_info.rows; y++)
              {
                if (offset != offsets[y+z*iris_info.rows])
                  {
                    offset=offsets[y+z*iris_info.rows];
                    (void) SeekBlob(image,(long) offset,SEEK_SET);
                  }
                (void) ReadBlob(image,runlength[y+z*iris_info.rows],
                  (char *) max_packets);
                if (EOFBlob(image))
                  {
                    ThrowReaderException(CorruptImageError,
                      UnexpectedEndOfFile, image);
                    break;
                  }
                offset+=runlength[y+z*iris_info.rows];
                SGIDecode(bytes_per_pixel,max_packets,p+bytes_per_pixel*z);
                p+=(iris_info.columns*4*bytes_per_pixel);
              }
            }
          }
        else
          {
            p=iris_pixels;
            for (y=0; y < (long) iris_info.rows; y++)
            {
              for (z=0; z < (int) iris_info.depth; z++)
              {
                if (offset != offsets[y+z*iris_info.rows])
                  {
                    offset=offsets[y+z*iris_info.rows];
                    (void) SeekBlob(image,(long) offset,SEEK_SET);
                  }
                (void) ReadBlob(image,runlength[y+z*iris_info.rows],
                  (char *) max_packets);
                if (EOFBlob(image))
                  {
                    ThrowReaderException(CorruptImageError,
                      UnexpectedEndOfFile, image);
                    break;
                  }
                offset+=runlength[y+z*iris_info.rows];
                SGIDecode(bytes_per_pixel,max_packets,p+bytes_per_pixel*z);
              }
              p+=(iris_info.columns*4*bytes_per_pixel);
              if (EOFBlob(image))
                break;
            }
          }
        MagickFreeMemory(runlength);
        MagickFreeMemory(max_packets);
        MagickFreeMemory(offsets);
      }
    /*
      Initialize image structure.
    */
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "   Initializing SGI image structure"); 
    image->matte=iris_info.depth == 4;
    image->columns=iris_info.columns;
    image->rows=iris_info.rows;
    /*
      Convert SGI raster image to pixel packets.
    */
    if (image->storage_class == DirectClass)
      {
        /*
          Convert SGI image to DirectClass pixel packets.
        */
        if (bytes_per_pixel == 2)
          {
            for (y=0; y < (long) image->rows; y++)
            {
              p=iris_pixels+(image->rows-y-1)*8*image->columns;
              q=SetImagePixels(image,0,y,image->columns,1);
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                q->red=ScaleShortToQuantum((*(p+0) << 8) | (*(p+1)));
                q->green=ScaleShortToQuantum((*(p+2) << 8) | (*(p+3)));
                q->blue=ScaleShortToQuantum((*(p+4) << 8) | (*(p+5)));
                if (image->matte)
                  q->opacity=(Quantum)
                    (MaxRGB-ScaleShortToQuantum((*(p+6) << 8) | (*(p+7))));
                else
                  q->opacity=OpaqueOpacity;
                p+=8;
                q++;
              }
              if (!SyncImagePixels(image))
                break;
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  if (!MagickMonitor(LoadImageText,y,image->rows,exception))
                    break;
            }
          }
        else
          for (y=0; y < (long) image->rows; y++)
          {
            p=iris_pixels+(image->rows-y-1)*4*image->columns;
            q=SetImagePixels(image,0,y,image->columns,1);
            if (q == (PixelPacket *) NULL)
              break;
            for (x=0; x < (long) image->columns; x++)
            {
              q->red=ScaleCharToQuantum(*p);
              q->green=ScaleCharToQuantum(*(p+1));
              q->blue=ScaleCharToQuantum(*(p+2));
              if (image->matte)
                q->opacity=(Quantum) (MaxRGB-ScaleCharToQuantum(*(p+3)));
              else
                q->opacity=OpaqueOpacity;
              p+=4;
              q++;
            }
            if (!SyncImagePixels(image))
              break;
            if (image->previous == (Image *) NULL)
              if (QuantumTick(y,image->rows))
                if (!MagickMonitor(LoadImageText,y,image->rows,exception))
                  break;
          }
      }
    else
      {
        /*
          Create grayscale map.
        */
        if (!AllocateImageColormap(image,image->colors))
          ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,
            image);
        /*
          Convert SGI image to PseudoClass pixel packets.
        */
        if (bytes_per_pixel == 2)
          {
            for (y=0; y < (long) image->rows; y++)
            {
              p=iris_pixels+(image->rows-y-1)*8*image->columns;
              q=SetImagePixels(image,0,y,image->columns,1);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=GetIndexes(image);
              for (x=0; x < (long) image->columns; x++)
              {
                indexes[x]=(*p << 8);
                indexes[x]|=(*(p+1));
                p+=8;
                q++;
              }
              if (!SyncImagePixels(image))
                break;
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  if (!MagickMonitor(LoadImageText,y,image->rows,exception))
                    break;
            }
          }
        else
          for (y=0; y < (long) image->rows; y++)
          {
            p=iris_pixels+(image->rows-y-1)*4*image->columns;
            q=SetImagePixels(image,0,y,image->columns,1);
            if (q == (PixelPacket *) NULL)
              break;
            indexes=GetIndexes(image);
            for (x=0; x < (long) image->columns; x++)
            {
              indexes[x]=(*p);
              p+=4;
              q++;
            }
            if (!SyncImagePixels(image))
              break;
            if (image->previous == (Image *) NULL)
              if (QuantumTick(y,image->rows))
                if (!MagickMonitor(LoadImageText,y,image->rows,exception))
                  break;
          }
        (void) SyncImage(image);
      }
    MagickFreeMemory(iris_pixels);
    if (EOFBlob(image))
      {
        ThrowReaderException(CorruptImageError,UnexpectedEndOfFile,
          image);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    iris_info.magic=ReadBlobMSBShort(image);
    if (iris_info.magic == 0x01DA)
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
  } while (iris_info.magic == 0x01DA);
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
%   R e g i s t e r S G I I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterSGIImage adds attributes for the SGI image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSGIImage method is:
%
%      RegisterSGIImage(void)
%
*/
ModuleExport void RegisterSGIImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("SGI");
  entry->decoder=(DecoderHandler) ReadSGIImage;
  entry->encoder=(EncoderHandler) WriteSGIImage;
  entry->magick=(MagickHandler) IsSGI;
  entry->description=AcquireString("Irix RGB image");
  entry->module=AcquireString("SGI");
  entry->seekable_stream=True;
  (void) RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S G I I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterSGIImage removes format registrations made by the
%  SGI module from the list of supported formats.
%
%  The format of the UnregisterSGIImage method is:
%
%      UnregisterSGIImage(void)
%
*/
ModuleExport void UnregisterSGIImage(void)
{
  (void) UnregisterMagickInfo("SGI");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S G I I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteSGIImage writes an image in SGI RGB encoded image format.
%
%  The format of the WriteSGIImage method is:
%
%      unsigned int WriteSGIImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteSGIImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o image:  A pointer to an Image structure.
%
%
*/

static size_t SGIEncode(unsigned char *pixels,size_t count,
  unsigned char *packets)
{
  short
    runlength;

  register unsigned char
    *p,
    *q;

  unsigned char
    *limit,
    *mark;

  p=pixels;
  limit=p+count*4;
  q=packets;
  while (p < limit)
  {
    mark=p;
    p+=8;
    while ((p < limit) && ((*(p-8) != *(p-4)) || (*(p-4) != *p)))
      p+=4;
    p-=8;
    count=((p-mark) >> 2);
    while (count)
    {
      runlength=(short) (count > 126 ? 126 : count);
      count-=runlength;
      *q++=0x80 | runlength;
      for ( ; runlength > 0; runlength--)
      {
        *q++=(*mark);
        mark+=4;
      }
    }
    mark=p;
    p+=4;
    while ((p < limit) && (*p == *mark))
      p+=4;
    count=((p-mark) >> 2);
    while (count)
    {
      runlength=(short) (count > 126 ? 126 : count);
      count-=runlength;
      *q++=(unsigned char) runlength;
      *q++=(*mark);
    }
  }
  *q++=0;
  return(q-packets);
}

static unsigned int WriteSGIImage(const ImageInfo *image_info,Image *image)
{
  long
    y,
    z;

  SGIInfo
    iris_info;

  register const PixelPacket
    *p;

  register long
    i,
    x;

  register unsigned char
    *q;

  unsigned char
    *iris_pixels,
    *packets;

  unsigned int
    status;

  unsigned long
    number_pixels,
    scene;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if ((image->columns > 65535L) || (image->rows > 65535L))
    ThrowWriterException(ImageError,WidthOrHeightExceedsLimit,image);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFail)
    ThrowWriterException(FileOpenError,UnableToOpenFile,image);
  scene=0;
  do
  {
    /*
      Initialize SGI raster file header.
    */
    (void) TransformColorspace(image,RGBColorspace);
    iris_info.magic=0x01DA;
    if (image_info->compression == NoCompression)
      iris_info.storage=0x00;
    else
      iris_info.storage=0x01;
    iris_info.bytes_per_pixel=1;  /* one byte per pixel */
    iris_info.dimension=3;
    iris_info.columns=(unsigned short) image->columns;
    iris_info.rows=(unsigned short) image->rows;
    if (image->matte != MagickFalse)
      iris_info.depth=4;
    else
      {
        if ((image_info->type != TrueColorType) &&
            (IsGrayImage(image,&image->exception) != False))
          {
            iris_info.dimension=2;
            iris_info.depth=1;
          }
        else
          iris_info.depth=3;
      }
    iris_info.minimum_value=0;
    iris_info.maximum_value=ScaleQuantumToChar(MaxRGB);
    for (i=0; i < (int) sizeof(iris_info.filler); i++)
      iris_info.filler[i]=0;
    /*
      Write SGI header.
    */
    (void) WriteBlobMSBShort(image,iris_info.magic);
    (void) WriteBlobByte(image,iris_info.storage);
    (void) WriteBlobByte(image,iris_info.bytes_per_pixel);
    (void) WriteBlobMSBShort(image,iris_info.dimension);
    (void) WriteBlobMSBShort(image,iris_info.columns);
    (void) WriteBlobMSBShort(image,iris_info.rows);
    (void) WriteBlobMSBShort(image,iris_info.depth);
    (void) WriteBlobMSBLong(image,iris_info.minimum_value);
    (void) WriteBlobMSBLong(image,iris_info.maximum_value);
    (void) WriteBlob(image,sizeof(iris_info.filler),
      (char *) iris_info.filler);
    /*
      Allocate SGI pixels.
    */
    number_pixels=image->columns*image->rows;
    iris_pixels=MagickAllocateMemory(unsigned char *,4*number_pixels);
    if (iris_pixels == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,image);
    /*
      Convert image pixels to uncompressed SGI pixels.
    */
    for (y=0; y < (long) image->rows; y++)
    {
      p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
      if (p == (const PixelPacket *) NULL)
        break;
      q=iris_pixels+((iris_info.rows-1)-y)*(iris_info.columns*4);
      for (x=0; x < (long) image->columns; x++)
      {
        *q++=ScaleQuantumToChar(p->red);
        *q++=ScaleQuantumToChar(p->green);
        *q++=ScaleQuantumToChar(p->blue);
        *q++=ScaleQuantumToChar(MaxRGB-p->opacity);
        p++;
      }
      if (image->previous == (Image *) NULL)
        if (QuantumTick(y,image->rows))
          if (!MagickMonitor(SaveImageText,y,image->rows,&image->exception))
            break;
    }
    if (image_info->compression == NoCompression)
      {
        unsigned char
          *scanline;

        /*
          Write uncompressed SGI pixels.
        */
        scanline=MagickAllocateMemory(unsigned char *,iris_info.columns);
        if (scanline == (unsigned char *) NULL)
          ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,
            image);
        for (z=0; z < (int) iris_info.depth; z++)
        {
          q=iris_pixels+z;
          for (y=0; y < (long) iris_info.rows; y++)
          {
            for (x=0; x < (long) iris_info.columns; x++)
            {
              scanline[x]=(*q);
              q+=4;
            }
            (void) WriteBlob(image,iris_info.columns,(char *) scanline);
          }
        }
        MagickFreeMemory(scanline);
      }
    else
      {
        unsigned long
          length,
          number_packets,
          offset,
          *offsets,
          *runlength;

        /*
          Convert SGI uncompressed pixels.
        */
        offsets=MagickAllocateMemory(unsigned long *,iris_info.rows*
          iris_info.depth*sizeof(unsigned long));
        packets=MagickAllocateMemory(unsigned char *,
          4*(2*iris_info.columns+10)*image->rows);
        runlength=MagickAllocateMemory(unsigned long *,iris_info.rows*
          iris_info.depth*sizeof(unsigned long));
        if ((offsets == (unsigned long *) NULL) ||
            (packets == (unsigned char *) NULL) ||
            (runlength == (unsigned long *) NULL))
          ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,
            image);
        offset=512+4*2*(iris_info.rows*iris_info.depth);
        number_packets=0;
        q=iris_pixels;
        for (y=0; y < (long) iris_info.rows; y++)
        {
          for (z=0; z < (int) iris_info.depth; z++)
          {
            length=
              SGIEncode(q+z,(int) iris_info.columns,packets+number_packets);
            number_packets+=length;
            offsets[y+z*iris_info.rows]=offset;
            runlength[y+z*iris_info.rows]=length;
            offset+=length;
          }
          q+=(iris_info.columns*4);
        }
        /*
          Write out line start and length tables and runlength-encoded pixels.
        */
        for (i=0; i < (int) (iris_info.rows*iris_info.depth); i++)
          (void) WriteBlobMSBLong(image,offsets[i]);
        for (i=0; i < (int) (iris_info.rows*iris_info.depth); i++)
          (void) WriteBlobMSBLong(image,runlength[i]);
        (void) WriteBlob(image,number_packets,(char *) packets);
        /*
          Free memory.
        */
        MagickFreeMemory(runlength);
        MagickFreeMemory(packets);
        MagickFreeMemory(offsets);
      }
    MagickFreeMemory(iris_pixels);
    if (image->next == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    if (!MagickMonitor(SaveImagesText,scene++,GetImageListLength(image),&image->exception))
      break;
  } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  CloseBlob(image);
  return(True);
}
