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
%                            RRRR   L       AAA                               %
%                            R   R  L      A   A                              %
%                            RRRR   L      AAAAA                              %
%                            R R    L      A   A                              %
%                            R  R   LLLLL  A   A                              %
%                                                                             %
%                                                                             %
%                     Read Alias/Wavefront Image Format.                      %
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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/cache.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d R L A I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadRLAImage reads a run-length encoded Wavefront RLA image file
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  Note:  This module was contributed by Lester Vecsey (master@internexus.net).
%
%  The format of the ReadRLAImage method is:
%
%      Image *ReadRLAImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadRLAImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadRLAImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  typedef struct _WindowFrame
  {
    short
      left,
      right,
      bottom,
      top;
  } WindowFrame;

  typedef struct _RLAInfo
  {
    WindowFrame
      window,
      active_window;

    short
      frame,
      storage_type,
      number_channels,
      number_matte_channels,
      number_auxiliary_channels,
      revision;

    char
      gamma[16],
      red_primary[24],
      green_primary[24],
      blue_primary[24],
      white_point[24];

    long
      job_number;

    char
      name[128],
      description[128],
      program[64],
      machine[32],
      user[32],
      date[20],
      aspect[24],
      aspect_ratio[8],
      chan[32];

    short
      field;

    char
      time[12],
      filter[32];

    short
      bits_per_channel,
      matte_type,
      matte_bits,
      auxiliary_type,
      auxiliary_bits;

    char
      auxiliary[32],
      space[36];

    long
      next;
  } RLAInfo;

  Image
    *image;

  int
    channel,
    length,
    runlength;

  long
    y;

  long
    *scanlines;

  register IndexPacket
    *indexes;

  register long
    i,
    x;

  register PixelPacket
    *q;

  RLAInfo
    rla_info;

  unsigned char
    byte;

  unsigned int
    status;

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
  rla_info.window.left=ReadBlobMSBShort(image);
  rla_info.window.right=ReadBlobMSBShort(image);
  rla_info.window.bottom=ReadBlobMSBShort(image);
  rla_info.window.top=ReadBlobMSBShort(image);
  rla_info.active_window.left=ReadBlobMSBShort(image);
  rla_info.active_window.right=ReadBlobMSBShort(image);
  rla_info.active_window.bottom=ReadBlobMSBShort(image);
  rla_info.active_window.top=ReadBlobMSBShort(image);
  rla_info.frame=ReadBlobMSBShort(image);
  rla_info.storage_type=ReadBlobMSBShort(image);
  rla_info.number_channels=ReadBlobMSBShort(image);
  rla_info.number_matte_channels=ReadBlobMSBShort(image);
  if (rla_info.number_channels == 0)
    rla_info.number_channels=3;
  rla_info.number_channels+=rla_info.number_matte_channels;
  rla_info.number_auxiliary_channels=ReadBlobMSBShort(image);
  rla_info.revision=ReadBlobMSBShort(image);
  (void) ReadBlob(image,16,(char *) rla_info.gamma);
  (void) ReadBlob(image,24,(char *) rla_info.red_primary);
  (void) ReadBlob(image,24,(char *) rla_info.green_primary);
  (void) ReadBlob(image,24,(char *) rla_info.blue_primary);
  (void) ReadBlob(image,24,(char *) rla_info.white_point);
  rla_info.job_number=(long) ReadBlobMSBLong(image);
  (void) ReadBlob(image,128,(char *) rla_info.name);
  (void) ReadBlob(image,128,(char *) rla_info.description);
  (void) ReadBlob(image,64,(char *) rla_info.program);
  (void) ReadBlob(image,32,(char *) rla_info.machine);
  (void) ReadBlob(image,32,(char *) rla_info.user);
  (void) ReadBlob(image,20,(char *) rla_info.date);
  (void) ReadBlob(image,24,(char *) rla_info.aspect);
  (void) ReadBlob(image,8,(char *) rla_info.aspect_ratio);
  (void) ReadBlob(image,32,(char *) rla_info.chan);
  rla_info.field=ReadBlobMSBShort(image);
  (void) ReadBlob(image,12,(char *) rla_info.time);
  (void) ReadBlob(image,32,(char *) rla_info.filter);
  rla_info.bits_per_channel=ReadBlobMSBShort(image);
  rla_info.matte_type=ReadBlobMSBShort(image);
  rla_info.matte_bits=ReadBlobMSBShort(image);
  rla_info.auxiliary_type=ReadBlobMSBShort(image);
  rla_info.auxiliary_bits=ReadBlobMSBShort(image);
  (void) ReadBlob(image,32,(char *) rla_info.auxiliary);
  (void) ReadBlob(image,36,(char *) rla_info.space);
  rla_info.next=(long) ReadBlobMSBLong(image);
  /*
    Initialize image structure.
  */
  image->matte=rla_info.number_matte_channels != 0;
  image->columns=rla_info.active_window.right-rla_info.active_window.left;
  image->rows=rla_info.active_window.top-rla_info.active_window.bottom;
  if (image_info->ping)
    {
      CloseBlob(image);
      return(image);
    }
  scanlines=MagickAllocateMemory(long *,image->rows*sizeof(long));
  if (scanlines == (long *) NULL)
    ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
  if (*rla_info.description != '\0')
    (void) SetImageAttribute(image,"comment",rla_info.description);
  /*
    Read offsets to each scanline data.
  */
  for (i=0; i < (long) image->rows; i++)
    scanlines[i]=(long) ReadBlobMSBLong(image);
  /*
    Read image data.
  */
  x=0;
  indexes=(IndexPacket *) NULL;
  for (y=0; y < (long) image->rows; y++)
  {
    (void) SeekBlob(image,scanlines[image->rows-y-1],SEEK_SET);
    for (channel=0; channel < (int) rla_info.number_channels; channel++)
    {
      length=ReadBlobMSBShort(image);
      while (length > 0)
      {
        byte=ReadBlobByte(image);
        runlength=byte;
        if (byte > 127U)
          runlength=byte-256;
        length--;
        if (length == 0)
          break;
        if (runlength < 0)
          {
            q=GetImagePixels(image,(long) (x % image->columns),
              (long) (y/image->columns),1,1);
            if (q == (PixelPacket *) NULL)
              break;
            indexes=GetIndexes(image);
            while (runlength < 0)
            {
              byte=ReadBlobByte(image);
              length--;
              switch (channel)
              {
                case 0:
                {
                  q->red=ScaleCharToQuantum(byte);
                  *indexes=0;
                  break;
                }
                case 1:
                {
                  q->green=ScaleCharToQuantum(byte);
                  break;
                }
                case 2:
                {
                  q->blue=ScaleCharToQuantum(byte);
                  break;
                }
                case 3:
                default:
                {
                  q->opacity=(Quantum) (MaxRGB-ScaleCharToQuantum(byte));
                  break;
                }
              }
              if (!SyncImagePixels(image))
                break;
              x++;
              runlength++;
            }
            continue;
          }
        byte=ReadBlobByte(image);
        length--;
        runlength++;
        do
        {
          q=GetImagePixels(image,(long) (x % image->columns),
            (long) (y/image->columns),1,1);
          if (q == (PixelPacket *) NULL)
            break;
          switch (channel)
          {
            case 0:
            {
              q->red=ScaleCharToQuantum(byte);
              *indexes=0;
              break;
            }
            case 1:
            {
              q->green=ScaleCharToQuantum(byte);
              break;
            }
            case 2:
            {
              q->blue=ScaleCharToQuantum(byte);
              break;
            }
            case 3:
            default:
            {
              q->opacity=(Quantum) (MaxRGB-ScaleCharToQuantum(byte));
              break;
            }
          }
          if (!SyncImagePixels(image))
            break;
          x++;
          runlength--;
        }
        while (runlength > 0);
      }
    }
    if (QuantumTick(y,image->rows))
      if (!MagickMonitor(LoadImageText,y,image->rows,exception))
        break;
  }
  if (EOFBlob(image))
    ThrowException(exception,CorruptImageError,UnexpectedEndOfFile,
      image->filename);
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r R L A I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterRLAImage adds attributes for the RLA image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterRLAImage method is:
%
%      RegisterRLAImage(void)
%
*/
ModuleExport void RegisterRLAImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("RLA");
  entry->decoder=(DecoderHandler) ReadRLAImage;
  entry->adjoin=False;
  entry->description=AcquireString("Alias/Wavefront image");
  entry->module=AcquireString("RLA");
  (void) RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r R L A I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterRLAImage removes format registrations made by the
%  RLA module from the list of supported formats.
%
%  The format of the UnregisterRLAImage method is:
%
%      UnregisterRLAImage(void)
%
*/
ModuleExport void UnregisterRLAImage(void)
{
  (void) UnregisterMagickInfo("RLA");
}
