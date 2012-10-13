/*
% Copyright (C) 2003 - 2011 GraphicsMagick Group
% Copyright (C) 2002 ImageMagick Studio
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        X   X  TTTTT  RRRR   N   N                           %
%                         X X     T    R   R  NN  N                           %
%                          X      T    RRRR   N N N                           %
%                         X X     T    R R    N  NN                           %
%                        X   X    T    R  R   N   N                           %
%                                                                             %
%                                                                             %
%                    ImageMagickObject BLOB Interface.                        %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                             William Radcliffe                               %
%                                 May 2001                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%  This coder is a kind of backdoor used by the COM object that allows it to  %
%  pass blobs back and forth using the coder interface. It simply encodes and %
%  decodes the filename as a comma delimited string and extracts the info it  %
%  needs. The five methods of passing images are:                             %
%                                                                             %
%     FILE   - same thing as filename so it should be a NOP                   %
%     IMAGE  - passes an image and image info structure                       %
%     BLOB   - passes binary blob containining the image                      %
%     ARRAY  - passes a pointer to a Win32 smart array and streams to it      %
%                                                                             %
%  Of all of these, the only one getting any real use at the moment is the    %
%  ARRAY handler. It is the primary way that images are shuttled back and     %
%  forth as blobs via COM since this is what VB and VBSCRIPT use internally   %
%  for this purpose.                                                          %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#if defined(_VISUALC_)
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/magick.h"
#include "magick/pixel_cache.h"
#include "magick/utility.h"
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <ole2.h>

/*
  Forward declarations.
*/
static unsigned int
  WriteXTRNImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X T R N I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadXTRNImage reads a XTRN image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadXTRNImage method is:
%
%      Image *ReadXTRNImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadXTRNImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadXTRNImage(const ImageInfo *image_info, ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *clone_info;

  void
    *param1,
    *param2,
    *param3;

  param1 = param2 = param3 = (void *) NULL;
  image = (Image *) NULL;
  clone_info=CloneImageInfo(image_info);
  if (clone_info->filename == NULL)
    {
      DestroyImageInfo(clone_info);
      ThrowReaderException(OptionError,MissingAnImageFilename,image);
    }
  /* DebugString("ReadXTRN CODER: %s\n",clone_info->filename); */
  if (LocaleCompare(image_info->magick,"XTRNFILE") == 0)
    {
      image=ReadImage(clone_info,exception);
      /* this should not be needed since the upstream code should catch any
         excetpions thrown by ReadImage
      */
      if (exception->severity != UndefinedException)
        MagickWarning2(exception->severity,exception->reason,exception->description);
    }
  else if (LocaleCompare(image_info->magick,"XTRNIMAGE") == 0)
    {
      Image
        **image_ptr;

#ifdef ALL_IMAGEINFO
      ImageInfo
        **image_info_ptr;
#endif

      (void) sscanf(clone_info->filename,"%lx,%lx",&param1,&param2);
      image_ptr=(Image **) param2;
      if (*image_ptr != (Image *)NULL)
        image=CloneImage(*image_ptr,0,0,False,&(*image_ptr)->exception);
#ifdef ALL_IMAGEINFO
      image_info_ptr=(ImageInfo **) param1;
      if (*image_info_ptr != (ImageInfo *)NULL)
        image_info=*image_info_ptr;
#endif
    }
  else if (LocaleCompare(image_info->magick,"XTRNBLOB") == 0)
    {
      char
        **blob_data;

      size_t
        *blob_length;

      char
        filename[MaxTextExtent];

      (void) sscanf(clone_info->filename,"%lx,%lx,%s",&param1,&param2,&filename);
      blob_data=(char **) param1;
      blob_length=(size_t *) param2;
      image=BlobToImage(clone_info,*blob_data,*blob_length,exception);
      if (exception->severity != UndefinedException)
        MagickWarning2(exception->severity,exception->reason,exception->description);
    }
  else if (LocaleCompare(image_info->magick,"XTRNARRAY") == 0)
    {
      SAFEARRAY
        *pSafeArray;

      char
        *blob_data;

      size_t
        blob_length;

      long
        lBoundl,
        lBoundu;

      HRESULT
        hr;

      char
        filename[MaxTextExtent];

      filename[0] = '\0';
      (void) sscanf(clone_info->filename,"%lx,%s",&param1,&filename);
      hr = S_OK;
      pSafeArray = (SAFEARRAY *) param1;
      if (pSafeArray)
        {
          hr = SafeArrayGetLBound(pSafeArray, 1, &lBoundl);
          if (SUCCEEDED(hr))
            hr = SafeArrayGetUBound(pSafeArray, 1, &lBoundu);
          if (SUCCEEDED(hr))
            {
              blob_length = lBoundu - lBoundl + 1;
              hr = SafeArrayAccessData(pSafeArray, (void**)&blob_data);
              if(SUCCEEDED(hr))
                {
                  if (filename[0] != '\0')
                    (void) strcpy(clone_info->filename, filename);
                  else
		    clone_info->filename[0] = '\0';
                  *clone_info->magick = '\0';
                  image=BlobToImage(clone_info,blob_data,blob_length,exception);
                  hr = SafeArrayUnaccessData(pSafeArray);
                  if (exception->severity != UndefinedException)
                    MagickWarning2(exception->severity,exception->reason,
				   exception->description);
                }
            }
        }
    }
  DestroyImageInfo(clone_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r X T R N I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterXTRNImage adds attributes for the XTRN image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterXTRNImage method is:
%
%      RegisterXTRNImage(void)
%
*/
ModuleExport void RegisterXTRNImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("XTRNFILE");
  entry->decoder=ReadXTRNImage;
  entry->encoder=WriteXTRNImage;
  entry->adjoin=False;
  entry->stealth=True;
  entry->description="External transfer of a file";
  entry->module="XTRN";
  RegisterMagickInfo(entry);

  entry=SetMagickInfo("XTRNIMAGE");
  entry->decoder=ReadXTRNImage;
  entry->encoder=WriteXTRNImage;
  entry->adjoin=False;
  entry->stealth=True;
  entry->description="External transfer of a image in memory";
  entry->module="XTRN";
  RegisterMagickInfo(entry);

  entry=SetMagickInfo("XTRNBLOB");
  entry->decoder=ReadXTRNImage;
  entry->encoder=WriteXTRNImage;
  entry->adjoin=False;
  entry->stealth=True;
  entry->description="IExternal transfer of a blob in memory";
  entry->module="XTRN";
  RegisterMagickInfo(entry);

  entry=SetMagickInfo("XTRNARRAY");
  entry->decoder=ReadXTRNImage;
  entry->encoder=WriteXTRNImage;
  entry->adjoin=False;
  entry->stealth=True;
  entry->description="External transfer via a smart array interface";
  entry->module="XTRN";
  RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r X T R N I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterXTRNImage removes format registrations made by the
%  XTRN module from the list of supported formats.
%
%  The format of the UnregisterXTRNImage method is:
%
%      UnregisterXTRNImage(void)
%
*/
ModuleExport void UnregisterXTRNImage(void)
{
  UnregisterMagickInfo("XTRNFILE");
  UnregisterMagickInfo("XTRNIMAGE");
  UnregisterMagickInfo("XTRNBLOB");
  UnregisterMagickInfo("XTRNARRAY");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X T R N I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteXTRNImage writes an image in the XTRN encoded image format.
%  We use GIF because it is the only format that is compressed without
%  requiring addition optional delegates (TIFF, ZIP, etc).
%
%  The format of the WriteXTRNImage method is:
%
%      unsigned int WriteXTRNImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteXTRNImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/

int SafeArrayFifo(const Image *image,const void *data,const size_t length)
{
  SAFEARRAYBOUND NewArrayBounds[1];  /* 1 Dimension */
  size_t tlen=length;
  SAFEARRAY *pSafeArray = (SAFEARRAY *)image->client_data;
  if (pSafeArray != NULL)
    {
      long lBoundl, lBoundu, lCount;
      HRESULT hr = S_OK;
      /* First see how big the buffer currently is */
      hr = SafeArrayGetLBound(pSafeArray, 1, &lBoundl);
      if (FAILED(hr))
	return MagickFalse;
      hr = SafeArrayGetUBound(pSafeArray, 1, &lBoundu);
      if (FAILED(hr))
	return MagickFalse;
      lCount = lBoundu - lBoundl + 1;

      if (length>0)
	{
	  unsigned char       *pReturnBuffer = NULL;
	  NewArrayBounds[0].lLbound = 0;   /* Start-Index 0 */
	  NewArrayBounds[0].cElements = (unsigned long) length+lCount;  /* # Elemente */
	  hr = SafeArrayRedim(pSafeArray, NewArrayBounds);
	  if (FAILED(hr))
	    return 0;
	  hr = SafeArrayAccessData(pSafeArray, (void**)&pReturnBuffer);
	  if( FAILED(hr) )
	    return 0;
	  memcpy( pReturnBuffer+lCount, (unsigned char *)data, length );
	  hr = SafeArrayUnaccessData(pSafeArray);
	  if( FAILED(hr) )
	    return 0;
	}
      else
	{
	  /* Adjust the length of the buffer to fit */
	}
    }
  return(tlen);
}

static unsigned int WriteXTRNImage(const ImageInfo *image_info,Image *image)
{
  Image *
    p;

  ImageInfo
    *clone_info;

  int
    scene;

  void
    *param1,
    *param2,
    *param3;

  unsigned int
    status;

  param1 = param2 = param3 = (void *) NULL;
  if (LocaleCompare(image_info->magick,"XTRNFILE") == 0)
    {
      clone_info=CloneImageInfo(image_info);
      status=WriteImage(image_info,image);
      if (status == False)
        CatchImageException(image);
      DestroyImageInfo(clone_info);
    }
  else if (LocaleCompare(image_info->magick,"XTRNIMAGE") == 0)
    {
      Image
        **image_ptr;

      ImageInfo
        **image_info_ptr;

      clone_info=CloneImageInfo(image_info);
      if (clone_info->filename[0])
        {
          (void) sscanf(clone_info->filename,"%lx,%lx",&param1,&param2);
          image_info_ptr=(ImageInfo **) param1;
          image_ptr=(Image **) param2;
          if ((image_info_ptr != (ImageInfo **) NULL) &&
              (image_ptr != (Image **) NULL))
            {
              *image_ptr=CloneImage(image,0,0,False,&(image->exception));
              *image_info_ptr=clone_info;
            }
        }
    }
  else if (LocaleCompare(image_info->magick,"XTRNBLOB") == 0)
    {
      char
        **blob_data;

      ExceptionInfo
        exception;

      size_t
        *blob_length;

      char
        filename[MaxTextExtent];

      clone_info=CloneImageInfo(image_info);
      if (clone_info->filename[0])
        {
          (void) sscanf(clone_info->filename,"%lx,%lx,%s",
			&param1,&param2,&filename);

          blob_data=(char **) param1;
          blob_length=(size_t *) param2;

          scene = 0;
          (void) strcpy(clone_info->filename, filename);
          for (p=image; p != (Image *) NULL; p=p->next)
	    {
	      (void) strcpy(p->filename, filename);
	      p->scene=scene++;
	    }
          SetImageInfo(clone_info,SETMAGICK_WRITE,&image->exception);
          (void) strcpy(image->magick,clone_info->magick);
          GetExceptionInfo(&exception);
          if (*blob_length == 0)
            *blob_length=8192;
          *blob_data=(char *) ImageToBlob(clone_info,image,blob_length,&exception);
          if (*blob_data == NULL)
            status=False;
          if (status == False)
            CatchImageException(image);
        }
      DestroyImageInfo(clone_info);
    }
  else if (LocaleCompare(image_info->magick,"XTRNARRAY") == 0)
    {
      char
        *blob_data;

      ExceptionInfo
        exception;

      size_t
        blob_length;

      char
        filename[MaxTextExtent];

      clone_info=CloneImageInfo(image_info);
      if (clone_info->filename[0])
        {
          (void) sscanf(clone_info->filename,"%lx,%s",
			&param1,&filename);

          image->client_data=param1;

          scene = 0;
	  blob_length = 0;
          (void) strcpy(clone_info->filename, filename);
          for (p=image; p != (Image *) NULL; p=p->next)
	    {
	      (void) strcpy(p->filename, filename);
	      p->scene=scene++;
	    }
          SetImageInfo(clone_info,SETMAGICK_WRITE,&image->exception);
          (void) strcpy(image->magick,clone_info->magick);
          GetExceptionInfo(&exception);
          blob_data=(char *) ImageToBlob(clone_info,image,&blob_length,&exception);
          if (blob_data == NULL)
            status=False;
          else
            SafeArrayFifo(image,blob_data,blob_length);

          if (status == False)
            CatchImageException(image);
        }
      DestroyImageInfo(clone_info);
    }
  return(True);
}
#endif /* defined(_VISUALC_) */
