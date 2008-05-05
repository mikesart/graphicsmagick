/*
% Copyright (C) 2003 GraphicsMagick Group
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
%                            X   X   CCCC  FFFFF                              %
%                             X X   C      F                                  %
%                              X    C      FFF                                %
%                             X X   C      F                                  %
%                            X   X   CCCC  F                                  %
%                                                                             %
%                                                                             %
%                       Read GIMP XCF Image Format.                           %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                              Leonard Rosenthol                              %
%                               November 2001                                 %
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
#include "magick/composite.h"
#include "magick/magick.h"
#include "magick/quantize.h"
#include "magick/utility.h"

/*
  Typedef declarations.
*/
typedef enum
{
  GIMP_RGB,
  GIMP_GRAY,
  GIMP_INDEXED
} GimpImageBaseType;

typedef enum
{
  PROP_END                   =  0,
  PROP_COLORMAP              =  1,
  PROP_ACTIVE_LAYER          =  2,
  PROP_ACTIVE_CHANNEL        =  3,
  PROP_SELECTION             =  4,
  PROP_FLOATING_SELECTION    =  5,
  PROP_OPACITY               =  6,
  PROP_MODE                  =  7,
  PROP_VISIBLE               =  8,
  PROP_LINKED                =  9,
  PROP_PRESERVE_TRANSPARENCY = 10,
  PROP_APPLY_MASK            = 11,
  PROP_EDIT_MASK             = 12,
  PROP_SHOW_MASK             = 13,
  PROP_SHOW_MASKED           = 14,
  PROP_OFFSETS               = 15,
  PROP_COLOR                 = 16,
  PROP_COMPRESSION           = 17,
  PROP_GUIDES                = 18,
  PROP_RESOLUTION            = 19,
  PROP_TATTOO                = 20,
  PROP_PARASITES             = 21,
  PROP_UNIT                  = 22,
  PROP_PATHS                 = 23,
  PROP_USER_UNIT             = 24
} PropType;

typedef enum
{
  COMPRESS_NONE              =  0,
  COMPRESS_RLE               =  1,
  COMPRESS_ZLIB              =  2,  /* unused */
  COMPRESS_FRACTAL           =  3   /* unused */
} XcfCompressionType;

typedef struct {
  magick_uint32_t
    width,
    height,
    image_type,
    bpp;  /* BYTES per pixel!! */

  int
    compression;

  /* not really part of the doc, but makes it easy to pass around! */
  ExceptionInfo 
    *exception;
} XCFDocInfo;

typedef struct {
  char
    name[1024];

  unsigned int
    active;

  magick_uint32_t
    width,
    height,
    type,
    opacity,
    visible,
    linked,
    preserve_trans,
    apply_mask,
    show_mask,
    edit_mask,
    floating_offset;

  magick_int32_t
    offset_x,
    offset_y;

  magick_uint32_t
    mode,
    tattoo;

  Image
    *image;
} XCFLayerInfo;

#define TILE_WIDTH   64
#define TILE_HEIGHT  64

typedef struct {
  unsigned char
    red,
    green,
    blue,
    opacity;  /* NOTE: reversed from IM! */
} XCFPixelPacket;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s X C F                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsXCF returns True if the image format type, identified by the
%  magick string, is XCF (GIMP native format).
%
%  The format of the IsXCF method is:
%
%      unsigned int IsXCF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsXCF returns True if the image format type is XCF.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static unsigned int IsXCF(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(False);
  if (LocaleNCompare((char *) magick,"gimp xcf",8) == 0)
    return(True);
  return(False);
}


typedef enum
{
  GIMP_NORMAL_MODE,
  GIMP_DISSOLVE_MODE,
  GIMP_BEHIND_MODE,
  GIMP_MULTIPLY_MODE,
  GIMP_SCREEN_MODE,
  GIMP_OVERLAY_MODE,
  GIMP_DIFFERENCE_MODE,
  GIMP_ADDITION_MODE,
  GIMP_SUBTRACT_MODE,
  GIMP_DARKEN_ONLY_MODE,
  GIMP_LIGHTEN_ONLY_MODE,
  GIMP_HUE_MODE,
  GIMP_SATURATION_MODE,
  GIMP_COLOR_MODE,
  GIMP_VALUE_MODE,
  GIMP_DIVIDE_MODE,
  GIMP_DODGE_MODE,
  GIMP_BURN_MODE,
  GIMP_HARDLIGHT_MODE
} GimpLayerModeEffects;

/*
  Simple utility routine to convert between PSD blending modes and
  GraphicsMagick compositing operators
*/
static CompositeOperator GIMPBlendModeToCompositeOperator( unsigned int blendMode )
{
  switch ( blendMode )
    {
    case GIMP_NORMAL_MODE:    return( OverCompositeOp );
    case GIMP_DISSOLVE_MODE:  return( DissolveCompositeOp );
    case GIMP_MULTIPLY_MODE:  return( MultiplyCompositeOp );
    case GIMP_SCREEN_MODE:    return( ScreenCompositeOp );
    case GIMP_OVERLAY_MODE:    return( OverlayCompositeOp );
    case GIMP_DIFFERENCE_MODE:  return( DifferenceCompositeOp );
    case GIMP_ADDITION_MODE:  return( AddCompositeOp );
    case GIMP_SUBTRACT_MODE:  return( SubtractCompositeOp );
    case GIMP_DARKEN_ONLY_MODE:  return( DarkenCompositeOp );
    case GIMP_LIGHTEN_ONLY_MODE:return( LightenCompositeOp );
    case GIMP_HUE_MODE:      return( HueCompositeOp );
    case GIMP_SATURATION_MODE:  return( SaturateCompositeOp );
    case GIMP_COLOR_MODE:    return( ColorizeCompositeOp );
      /* these are the ones we don't support...yet */
    case GIMP_DODGE_MODE:    return( OverCompositeOp );
    case GIMP_BURN_MODE:    return( OverCompositeOp );
    case GIMP_HARDLIGHT_MODE:  return( OverCompositeOp );
    case GIMP_BEHIND_MODE:    return( OverCompositeOp );
    case GIMP_VALUE_MODE:    return( OverCompositeOp );
    case GIMP_DIVIDE_MODE:    return( OverCompositeOp );
    default:
      return( OverCompositeOp );
    }
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d B l o b S t r i n g W i t h L o n g S i z e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadBlobStringWithLongSize reads characters from a blob or file
%  starting with a long length byte and then characters to that length
%
%  The format of the ReadBlobStringWithLongSize method is:
%
%      char *ReadBlobStringWithLongSize(Image *image,char *string)
%
%  A description of each parameter follows:
%
%    o status:  Method ReadBlobString returns the string on success, otherwise,
%      a null is returned.
%
%    o image: The image.
%
%    o string: The address of a character buffer.
%
%    o max: Length of 'string' array.
%
%
*/
static char *ReadBlobStringWithLongSize(Image *image,char *string,size_t max)
{
  int
    c;

  register unsigned long
    i;

  unsigned long
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(max != 0);
  length = ReadBlobMSBLong(image);
  for (i=0; i < Min(length,max-1); i++)
    {
      c=ReadBlobByte(image);
      if (c == EOF)
        return((char *) NULL);
      string[i]=c;
    }
  string[i]='\0';
  (void) SeekBlob(image, length-i, SEEK_CUR);
  return(string);
}


static int load_tile (Image* image, Image* tile_image, XCFDocInfo* inDocInfo, 
                      XCFLayerInfo*  inLayerInfo, size_t data_length)
{
  int
    i,
    nmemb_read_successfully;

  PixelPacket
    *q;

  XCFPixelPacket
    *xcfdata,
    *xcfodata;

  unsigned char
    *graydata;

  xcfdata = xcfodata = MagickAllocateMemory(XCFPixelPacket *,data_length);
  graydata = (unsigned char *)xcfdata;  /* used by gray and indexed */
  nmemb_read_successfully = ReadBlob(image, data_length, xcfdata);

  q=SetImagePixels(tile_image,0,0,tile_image->columns,tile_image->rows);

  /* we have to copy the pixels individually since IM uses a different
     format PixelPacket on different platforms - not to mention
     different QuantumDepths!
  */
  for( i=0; i<nmemb_read_successfully;i++ )
    {
      if ( inDocInfo->image_type == GIMP_GRAY )
        {
          q->red      = ScaleCharToQuantum(*graydata);
          q->green    = ScaleCharToQuantum(*graydata);
          q->blue     = ScaleCharToQuantum(*graydata);
          q->opacity  = ScaleCharToQuantum(255-inLayerInfo->opacity);
          graydata++;
        }
      else
        if ( inDocInfo->image_type == GIMP_RGB )
          {
            q->red      = ScaleCharToQuantum(xcfdata->red);
            q->green    = ScaleCharToQuantum(xcfdata->green);
            q->blue     = ScaleCharToQuantum(xcfdata->blue);
            q->opacity  = (Quantum) (xcfdata->opacity==0 ? TransparentOpacity :
                                     ScaleCharToQuantum(255-inLayerInfo->opacity)); 

            xcfdata++;
          };

      q++;
    }

  MagickFreeMemory(xcfodata);
  return True;
}

static int load_tile_rle (Image* image, Image* tile_image, XCFDocInfo* inDocInfo, 
                          XCFLayerInfo*  inLayerInfo, size_t data_length)
{
  unsigned char
    data,
    val;

  ExtendedSignedIntegralType
    size;

  int
    count,
    length,
    bpp,    /* BYTES per pixel! */
    i,
    j,
    nmemb_read_successfully;

  unsigned char
    *xcfdata,
    *xcfodata,
    *xcfdatalimit;

  PixelPacket
    *q;

  bpp = (int) inDocInfo->bpp;
 
  xcfdata = xcfodata = MagickAllocateMemory(unsigned char *,data_length);

  nmemb_read_successfully = ReadBlob(image, data_length, xcfdata);

  xcfdatalimit = &xcfodata[nmemb_read_successfully - 1];
  
  for (i = 0; i < bpp; i++)
    {
      q=SetImagePixels(tile_image,0,0,tile_image->columns,tile_image->rows);
      size = tile_image->rows * tile_image->columns;
      count = 0;

      while (size > 0)
        {
          if (xcfdata > xcfdatalimit)
            {
              goto bogus_rle;
            }
    
          val = *xcfdata++;
    
          length = val;

          if (length >= 128)
            {
              length = 255 - (length - 1);

              if (length == 128)
                {
                  if (xcfdata >= xcfdatalimit)
                    {
                      goto bogus_rle;
                    }
    
                  length = (*xcfdata << 8) + xcfdata[1];
                  xcfdata += 2;
                }
    
              count += length;
              size -= length;
    
              if (size < 0)
                {
                  goto bogus_rle;
                }
    
              if (&xcfdata[length-1] > xcfdatalimit)
                {
                  goto bogus_rle;
                }
    
              while (length-- > 0)
                {
                  data = *xcfdata++;
                  switch (i) {
                  case 0:  
                    q->red          = ScaleCharToQuantum(data);  
                    if ( inDocInfo->image_type == GIMP_GRAY )
                      {
                        q->green    = ScaleCharToQuantum(data);
                        q->blue     = ScaleCharToQuantum(data);
                        q->opacity  = ScaleCharToQuantum(255-inLayerInfo->opacity);
                      }
                    else
                      {
                        q->green    = q->red;
                        q->blue     = q->red;
                        q->opacity  = ScaleCharToQuantum(255-inLayerInfo->opacity);
                      }
                    break;
                  case 1:  q->green = ScaleCharToQuantum(data);  break;
                  case 2:  q->blue  = ScaleCharToQuantum(data);  break;
                  case 3:  
                    q->opacity = (Quantum) (data==0 ? TransparentOpacity :
                                            ScaleCharToQuantum(255-inLayerInfo->opacity));  
                    break;
                  }
                  q++;
                }
            }
          else
            {
              length += 1;
              if (length == 128)
                {
                  if (xcfdata >= xcfdatalimit)
                    {
                      goto bogus_rle;
                    }
    
                  length = (*xcfdata << 8) + xcfdata[1];
                  xcfdata += 2;
                }
    
              count += length;
              size -= length;
    
              if (size < 0)
                {
                  goto bogus_rle;
                }
    
              if (xcfdata > xcfdatalimit)
                {
                  goto bogus_rle;
                }
    
              val = *xcfdata++;
    
              for (j = 0; j < length; j++)
                {
                  data = val;
                  switch (i) {
                  case 0:  
                    q->red    = ScaleCharToQuantum(data);  
                    if ( inDocInfo->image_type == GIMP_GRAY )
                      {
                        q->green  = ScaleCharToQuantum(data);
                        q->blue    = ScaleCharToQuantum(data);
                        q->opacity  = ScaleCharToQuantum(255-inLayerInfo->opacity);
                      }
                    else
                      {
                        q->green  = q->red;
                        q->blue    = q->red;
                        q->opacity  = ScaleCharToQuantum(255-inLayerInfo->opacity);
                      }
                    break;
                  case 1:  q->green  = ScaleCharToQuantum(data);  break;
                  case 2:  q->blue    = ScaleCharToQuantum(data);  break;
                  case 3:  
                    q->opacity = (Quantum) (data==0 ? TransparentOpacity :
                                            ScaleCharToQuantum(255-inLayerInfo->opacity));  
                    break;
                  }
                  q++;
                }
            }
        }
    }
  MagickFreeMemory(xcfodata);
  return True;

 bogus_rle:
  if (xcfodata)
    MagickFreeMemory(xcfodata);
  return False;
}

static int load_level (Image* image, XCFDocInfo* inDocInfo, XCFLayerInfo*
                       inLayerInfo)
{
  ExtendedSignedIntegralType
    saved_pos,
    offset,
    offset2;

  unsigned long
    ntiles,
    ntile_rows,
    ntile_cols;

  int
    i,
    fail;

  Image*
    tile_image;

  int
    destLeft = 0,
    destTop = 0,
    tile_image_width,
    tile_image_height;

  ExceptionInfo 
    *exception = inDocInfo->exception;
  
  /* start reading the data */
  (void) ReadBlobMSBLong(image); /* width */
  (void) ReadBlobMSBLong(image); /* height */

  /* read in the first tile offset.
   *  if it is '0', then this tile level is empty
   *  and we can simply return.
   */
  offset = ReadBlobMSBLong(image);
  if (offset == 0)
    return 1;

  /* Initialise the reference for the in-memory tile-compression
   */
  ntile_rows = (inDocInfo->height + TILE_HEIGHT - 1) / TILE_HEIGHT;
  ntile_cols = (inDocInfo->width  + TILE_WIDTH  - 1) / TILE_WIDTH;
  ntiles = ntile_rows * ntile_cols;
  for (i = 0; i < (long) ntiles; i++)
    {
      fail = False;

      if (offset == 0)
        ThrowBinaryException(CorruptImageError,NotEnoughTiles,image->filename);

      /* save the current position as it is where the
       *  next tile offset is stored.
       */
      saved_pos = TellBlob(image);

      /* read in the offset of the next tile so we can calculate the amount
         of data needed for this tile*/
      offset2 = ReadBlobMSBLong(image);
      
      /* if the offset is 0 then we need to read in the maximum possible
         allowing for negative compression */
      if (offset2 == 0)
        offset2 = (unsigned long) (offset + TILE_WIDTH * TILE_WIDTH * 4* 1.5); 
      /* 1.5 is probably more
         than we need to allow */

      /* seek to the tile offset */
      (void) SeekBlob(image, offset, SEEK_SET);

      /* allocate the image for the tile 
         NOTE: the last tile in a row or column may not be a full tile!
      */
      tile_image_width = destLeft==(int) ntile_cols-1 ?
        (int) image->columns % TILE_WIDTH :  TILE_WIDTH;
      if (tile_image_width == 0)  tile_image_width = TILE_WIDTH;
      tile_image_height = destTop==(int) ntile_rows-1 ?
        (int) image->rows % TILE_HEIGHT :  TILE_HEIGHT;
      if (tile_image_height == 0)  tile_image_height = TILE_HEIGHT;
      tile_image=CloneImage(inLayerInfo->image, tile_image_width,
                            tile_image_height,True,exception);

      /* read in the tile */
      switch (inDocInfo->compression)
        {
        case COMPRESS_NONE:
          if (!load_tile (image,tile_image,inDocInfo,inLayerInfo,(size_t) (offset2-offset)))
            fail = True;
          break;
        case COMPRESS_RLE:
          if (!load_tile_rle (image,tile_image,inDocInfo,inLayerInfo,
                              (size_t) (offset2-offset)))
            fail = True;
          break;
        case COMPRESS_ZLIB:
          ThrowBinaryException(CoderError,ZipCompressionNotSupported,
                               image->filename)
            case COMPRESS_FRACTAL:
          ThrowBinaryException(CoderError,FractalCompressionNotSupported,
                               image->filename)
            }

      /* composite the tile onto the layer's image, and then destroy it */
      (void) CompositeImage(inLayerInfo->image,CopyCompositeOp,tile_image,
                            destLeft * TILE_WIDTH,destTop*TILE_HEIGHT);
      DestroyImage(tile_image);

      /* adjust tile position */
      destLeft++;
      if (destLeft >= (int) ntile_cols)
        {
          destLeft = 0;
          destTop++;
        }

      if (fail) 
        return False;
      
      /* restore the saved position so we'll be ready to
       *  read the next offset.
       */
      (void) SeekBlob(image, saved_pos, SEEK_SET);

      /* read in the offset of the next tile */
      offset = ReadBlobMSBLong(image);
    }


  if (offset != 0)
    ThrowBinaryException(CorruptImageError,CorruptImage,image->filename)

      return 1;
}

static int load_hierarchy (Image *image, XCFDocInfo* inDocInfo, XCFLayerInfo*
                           inLayer)
{
  ExtendedSignedIntegralType
    saved_pos,
    offset,
    junk;

  (void) ReadBlobMSBLong(image); /* width */
  (void) ReadBlobMSBLong(image); /* height */
  inDocInfo->bpp = ReadBlobMSBLong(image); /* bpp */

  /* load in the levels...we make sure that the number of levels
   *  calculated when the TileManager was created is the same
   *  as the number of levels found in the file.
   */
  offset = ReadBlobMSBLong(image);  /* top level */

  /* discard offsets for layers below first, if any.
   */
  do 
    {
      junk = ReadBlobMSBLong(image);
    }
  while (junk != 0);

  /* save the current position as it is where the
   *  next level offset is stored.
   */
  saved_pos = TellBlob(image);
  
  /* seek to the level offset */
  (void) SeekBlob(image, offset, SEEK_SET);
  
  /* read in the level */
  if (!load_level (image, inDocInfo, inLayer))
    return 0;
      
  /* restore the saved position so we'll be ready to
   *  read the next offset.
   */
  (void) SeekBlob(image, saved_pos, SEEK_SET);

  return 1;
}


static int ReadOneLayer( Image* image, XCFDocInfo* inDocInfo, XCFLayerInfo*
                         outLayer )
{
  int
    i;

  unsigned int
    foundPropEnd = 0;

  unsigned long
    hierarchy_offset, 
    layer_mask_offset;

  /* clear the block! */
  (void) memset( outLayer, 0, sizeof( XCFLayerInfo ) );
  
  /* read in the layer width, height, type and name */
  outLayer->width = ReadBlobMSBLong(image);
  outLayer->height = ReadBlobMSBLong(image);
  outLayer->type = ReadBlobMSBLong(image);
  (void) ReadBlobStringWithLongSize(image, outLayer->name,
                                    sizeof(outLayer->name));

  /* allocate the image for this layer */
  outLayer->image=CloneImage(image,outLayer->width, outLayer->height,True,
                             &image->exception);
  if (outLayer->image == (Image *) NULL)
    return False;

  /* read the layer properties! */
  foundPropEnd = 0;
  while ( !foundPropEnd && !EOFBlob(image) ) {
    PropType    prop_type = (PropType) ReadBlobMSBLong(image);
    unsigned long  prop_size = ReadBlobMSBLong(image);

    switch (prop_type)
      {
      case PROP_END:
        foundPropEnd = 1;
        break;
      case PROP_ACTIVE_LAYER:
        outLayer->active = 1;
        break;
      case PROP_FLOATING_SELECTION:
        outLayer->floating_offset = ReadBlobMSBLong(image);
        break;
      case PROP_OPACITY:
        outLayer->opacity = ReadBlobMSBLong(image);
        break;
      case PROP_VISIBLE:
        outLayer->visible = ReadBlobMSBLong(image);
        break;
      case PROP_LINKED:
        outLayer->linked = ReadBlobMSBLong(image);
        break;
      case PROP_PRESERVE_TRANSPARENCY:
        outLayer->preserve_trans = ReadBlobMSBLong(image);
        break;
      case PROP_APPLY_MASK:
        outLayer->apply_mask = ReadBlobMSBLong(image);
        break;
      case PROP_EDIT_MASK:
        outLayer->edit_mask = ReadBlobMSBLong(image);
        break;
      case PROP_SHOW_MASK:
        outLayer->show_mask = ReadBlobMSBLong(image);
        break;
      case PROP_OFFSETS:
        outLayer->offset_x = (magick_int32_t) ReadBlobMSBLong(image);
        outLayer->offset_y = (magick_int32_t) ReadBlobMSBLong(image);
        break;
      case PROP_MODE:
        outLayer->mode = ReadBlobMSBLong(image);
        break;
      case PROP_TATTOO:
        outLayer->preserve_trans = ReadBlobMSBLong(image);
        break;
      case PROP_PARASITES:
        {
          for (i=0; i<(long) prop_size; i++ )
            (void) ReadBlobByte(image);

          /*
            long base = info->cp;
            GimpParasite *p;
            while (info->cp - base < prop_size)
            {
            p = xcf_load_parasite(info);
            gimp_drawable_parasite_attach(GIMP_DRAWABLE(layer), p);
            gimp_parasite_free(p);
            }
            if (info->cp - base != prop_size)
            g_message ("Error detected while loading a layer's parasites");
          */
        }
        break;
      default:
        /* g_message ("unexpected/unknown layer property: %d (skipping)",
           prop_type); */

        {
          int buf[16];
          unsigned int amount;

          /* read over it... */
          while (prop_size > 0 && !EOFBlob(image))
            {
              amount = Min (16, prop_size);
              for (i=0; i<(long)amount; i++)
                amount = ReadBlob(image, amount, &buf);
              prop_size -= Min (16, amount);
            }
        }
        break;
      }
  }

  if (!foundPropEnd)
    return False;

  /* clear the image based on the layer opacity */
  (void) SetImage(outLayer->image,(Quantum)(255-outLayer->opacity));

  /* set the compositing mode */
  outLayer->image->compose = GIMPBlendModeToCompositeOperator( outLayer->mode );
  if ( outLayer->visible == False )
    {
      /* BOGUS: should really be separate member var! */
      outLayer->image->compose = NoCompositeOp;  
    }

  /* read the hierarchy and layer mask offsets */
  hierarchy_offset = ReadBlobMSBLong(image);
  layer_mask_offset = ReadBlobMSBLong(image);

  /* read in the hierarchy */
  (void) SeekBlob(image, hierarchy_offset, SEEK_SET);
  if (!load_hierarchy (image, inDocInfo, outLayer))
    return 0;

  /* read in the layer mask */
  if (layer_mask_offset != 0)
    {
      (void) SeekBlob(image, layer_mask_offset, SEEK_SET);

#if 0  /* BOGUS: support layer masks! */
      layer_mask = xcf_load_layer_mask (info, gimage);
      if (!layer_mask)
        goto error;

      /* set the offsets of the layer_mask */
      GIMP_DRAWABLE (layer_mask)->offset_x = GIMP_DRAWABLE (layer)->offset_x;
      GIMP_DRAWABLE (layer_mask)->offset_y = GIMP_DRAWABLE (layer)->offset_y;

      gimp_layer_add_mask (layer, layer_mask, False);

      layer->mask->apply_mask = apply_mask;
      layer->mask->edit_mask  = edit_mask;
      layer->mask->show_mask  = show_mask;
#endif
    }

  /* attach the floating selection... */
#if 0  /* BOGUS: we may need to read this, even if we don't support it! */
  if (add_floating_sel)
    {
      GimpLayer *floating_sel;

      floating_sel = info->floating_sel;
      floating_sel_attach (floating_sel, GIMP_DRAWABLE (layer));
    }
#endif

  return True;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X C F I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadXCFImage reads a GIMP (GNU Image Manipulation Program) image
%  file and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadXCFImage method is:
%
%      image=ReadXCFImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadXCFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadXCFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    magick[14];

  Image
    *image;

  unsigned int
    status;

  unsigned long
    i,
    image_type;

  int
    foundPropEnd = 0;

  size_t
    count;

  XCFDocInfo
    doc_info;

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
  count=ReadBlob(image,14,(char *) magick);
  if ((count == 0) ||
      (LocaleNCompare((char *) magick,"gimp xcf",8) != 0))
    ThrowReaderException(CorruptImageError,ImproperImageHeader,image);
  /* clear the docinfo stuff */
  (void) memset( &doc_info, 0, sizeof(XCFDocInfo));
  doc_info.exception = exception;

  /* read the three simple values */
  image->columns = doc_info.width = ReadBlobMSBLong(image);
  image->rows = doc_info.height = ReadBlobMSBLong(image);
  image_type = doc_info.image_type = ReadBlobMSBLong(image);
  
  /* setup some things about the image...*/
  image->compression=NoCompression;
  image->depth = 8;
  if ( image_type == GIMP_RGB ) {
    image->colorspace=RGBColorspace;
  } else if ( image_type == GIMP_GRAY ) {
    image->colorspace=GRAYColorspace;
  } else if ( image_type == GIMP_INDEXED )
    ThrowReaderException(CoderError,ColormapTypeNotSupported,image);
  (void) SetImage(image,OpaqueOpacity);  /* until we know otherwise...*/
  image->matte=True;  /* XCF always has a matte! */

  /* read properties */
  while ( !foundPropEnd && !EOFBlob(image) ) {
    PropType    prop_type = (PropType) ReadBlobMSBLong(image);
    unsigned long  prop_size = ReadBlobMSBLong(image);

    switch ( prop_type ) {
    case PROP_END:
      foundPropEnd = 1;

    case PROP_COLORMAP:
      /* BOGUS: just skip it for now */
      for (i=0; i<prop_size; i++ )
        (void) ReadBlobByte(image);
      /*
        if (info->file_version == 0) 
        {
        gint i;

        g_message (_("XCF warning: version 0 of XCF file format\n"
        "did not save indexed colormaps correctly.\n"
        "Substituting grayscale map."));
        info->cp += 
        xcf_read_int32 (info->fp, (guint32*) &gimage->num_cols, 1);
        gimage->cmap = g_new (guchar, gimage->num_cols*3);
        xcf_seek_pos (info, info->cp + gimage->num_cols);
        for (i = 0; i<gimage->num_cols; i++) 
        {
        gimage->cmap[i*3+0] = i;
        gimage->cmap[i*3+1] = i;
        gimage->cmap[i*3+2] = i;
        }
        }
        else 
        {
        info->cp += 
        xcf_read_int32 (info->fp, (guint32*) &gimage->num_cols, 1);
        gimage->cmap = g_new (guchar, gimage->num_cols*3);
        info->cp += 
        xcf_read_int8 (info->fp, 
        (guint8*) gimage->cmap, gimage->num_cols*3);
        }
      */
      break;

    case PROP_COMPRESSION:
      {
        doc_info.compression = ReadBlobByte(image);

        if ((doc_info.compression != COMPRESS_NONE) &&
            (doc_info.compression != COMPRESS_RLE) &&
            (doc_info.compression != COMPRESS_ZLIB) &&
            (doc_info.compression != COMPRESS_FRACTAL))
          ThrowReaderException(CorruptImageWarning,CompressionNotValid,image);
      }
      break;

    case PROP_GUIDES:
      {
        /* just skip it - we don't care about guides */
        for (i=0; i<prop_size; i++ )
          (void) ReadBlobByte(image);
      }
      break;

    case PROP_RESOLUTION:
      {
        /* float xres = (float) */ (void) ReadBlobMSBLong(image);
        /* float yres = (float) */ (void) ReadBlobMSBLong(image);

        /*
          if (xres < GIMP_MIN_RESOLUTION || xres > GIMP_MAX_RESOLUTION ||
          yres < GIMP_MIN_RESOLUTION || yres > GIMP_MAX_RESOLUTION)
          {
          g_message ("Warning, resolution out of range in XCF file");
          xres = gimage->gimp->config->default_xresolution;
          yres = gimage->gimp->config->default_yresolution;
          }
        */

      
        /* BOGUS: we don't write these yet because we aren't
        //      reading them properly yet :(
        //image->x_resolution = xres;
        //image->y_resolution = yres;
        */
      }
      break;

    case PROP_TATTOO:
      {
        /* we need to read it, even if we ignore it */
        /*unsigned long  tattoo_state = */ (void) ReadBlobMSBLong(image);
      }
      break;

    case PROP_PARASITES:
      {
        /* BOGUS: we may need these for IPTC stuff */
        for (i=0; i<prop_size; i++ )
          (void) ReadBlobByte(image);

        /*
          glong         base = info->cp;
          GimpParasite *p;

          while (info->cp - base < prop_size)
          {
          p = xcf_load_parasite (info);
          gimp_image_parasite_attach (gimage, p);
          gimp_parasite_free (p);
          }
          if (info->cp - base != prop_size)
          g_message ("Error detected while loading an image's parasites");
        */
      }
      break;

    case PROP_UNIT:
      {
        /* BOGUS: ignore for now... */
        /*unsigned long unit =  */ (void) ReadBlobMSBLong(image);
      }
      break;

    case PROP_PATHS:
      {
        /* BOGUS: just skip it for now */
        for (i=0; i<prop_size; i++ )
          (void) ReadBlobByte(image);

        /*
          PathList *paths = xcf_load_bzpaths (gimage, info);
          gimp_image_set_paths (gimage, paths);
        */
      }
      break;

    case PROP_USER_UNIT:
      {
        char  unit_string[1000];
        /*BOGUS: ignored for now */
        /*float  factor = (float) */ (void) ReadBlobMSBLong(image);
        /* unsigned long digits =  */ (void) ReadBlobMSBLong(image);
        for (i=0; i<5; i++)
          (void) ReadBlobStringWithLongSize(image, unit_string,
                                            sizeof(unit_string));
      }
      break;

    default:
      /* g_message ("unexpected/unknown image property: %d (skipping)",
         prop_type); */

      {
        int buf[16];
        long amount;

        /* read over it... */
        while (prop_size > 0 && !EOFBlob(image))
          {
            amount = (long) Min (16, prop_size);
            for (i=0; i<(unsigned long) amount; i++)
              amount = (long) ReadBlob(image, amount, &buf);
            prop_size -= Min (16, amount);
          }
      }
      break;
    }
  }

  if (!foundPropEnd)
    ThrowReaderException(CorruptImageError,ImproperImageHeader,image);

  if (image_info->ping && (image_info->subrange != 0)) {
    ; /* do nothing, we were just pinging! */
  } else {
    XCFLayerInfo*  layer_info;
    int  number_layers = 0,
      current_layer = 0,
      foundAllLayers = False;

    /* BIG HACK
       because XCF doesn't include the layer count, and we
       want to know it in advance in order to allocate memory,
       we have to scan the layer offset list, and then reposition
       the read pointer
    */
    ExtendedSignedIntegralType  oldPos = TellBlob(image);
    do {
      long  offset = (long) ReadBlobMSBLong(image);
      if ( offset == 0 )
        foundAllLayers = True;
      else
        number_layers++;
    } while ( !foundAllLayers );
    (void) SeekBlob(image, oldPos, SEEK_SET); /* restore the position! */


    /* allocate our array of layer info blocks */
    layer_info=MagickAllocateArray(XCFLayerInfo *,
                                   number_layers,
                                   sizeof(XCFLayerInfo));
    if (layer_info == (XCFLayerInfo *) NULL)
      ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
    (void) memset(layer_info,0,number_layers*sizeof(XCFLayerInfo));

    for ( ; ; )
      {
        ExtendedSignedIntegralType
          offset,
          saved_pos;
        int
          layer_ok;

        /* read in the offset of the next layer */
        offset = ReadBlobMSBLong(image);

        /* if the offset is 0 then we are at the end
         *  of the layer list.
         */
        if (offset == 0)
          break;

        /* save the current position as it is where the
         *  next layer offset is stored.
         */
        saved_pos = TellBlob(image);

        /* seek to the layer offset */
        (void) SeekBlob(image, offset, SEEK_SET);

        /* read in the layer */
        layer_ok = ReadOneLayer( image, &doc_info, &layer_info[current_layer] );
        if (!layer_ok) {
          int j;
          for (j=0; j < current_layer; j++)
            DestroyImage(layer_info[j].image);
          ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image)
            }

        /* restore the saved position so we'll be ready to
         *  read the next offset.
         */
        (void) SeekBlob(image, saved_pos, SEEK_SET);

        current_layer++;
      }

    if ( number_layers == 1 ) {
      /* composite the layer data onto the main image & then dispose the layer */
      (void) CompositeImage(image, OverCompositeOp, layer_info[0].image, 
                            layer_info[0].offset_x, layer_info[0].offset_y );
      DestroyImage( layer_info[0].image );

      /* Bob says that if we do this, we'll get REAL gray images! */
      if ( image_type == GIMP_GRAY ) {
        QuantizeInfo  qi;
        GetQuantizeInfo(&qi);
        qi.colorspace = GRAYColorspace;
        (void) QuantizeImage( &qi, image );
      }
    } else {
#if 0
      {
        /* NOTE: XCF layers are REVERSED from composite order! */
        signed int  j;
        for (j=number_layers-1; j>=0; j--) {
          /* BOGUS: need to consider layer blending modes!! */
          
          if ( layer_info[j].visible ) { /* only visible ones, please! */
            CompositeImage(image, OverCompositeOp, layer_info[j].image, 
                           layer_info[j].offset_x, layer_info[j].offset_y );
            DestroyImage( layer_info[j].image );

            /* Bob says that if we do this, we'll get REAL gray images! */
            if ( image_type == GIMP_GRAY ) {
              QuantizeInfo  qi;
              GetQuantizeInfo(&qi);
              qi.colorspace = GRAYColorspace;
              QuantizeImage( &qi, layer_info[j].image );
            }
          }
        }
      }
#else
      {
        /* NOTE: XCF layers are REVERSED from composite order! */
        signed int  j;

        /* first we copy the last layer on top of the main image */
        (void) CompositeImage(image, CopyCompositeOp, layer_info[number_layers-1].image, 
                              layer_info[number_layers-1].offset_x, 
                              layer_info[number_layers-1].offset_y );
        DestroyImage( layer_info[number_layers-1].image );

        /* Bob says that if we do this, we'll get REAL gray images! */
        if ( image_type == GIMP_GRAY ) {
          QuantizeInfo  qi;
          GetQuantizeInfo(&qi);
          qi.colorspace = GRAYColorspace;
          (void) QuantizeImage( &qi, image );
        }

        /* now reverse the order of the layers as they are put
           into subimages
        */
        image->next=layer_info[number_layers-2].image;
        layer_info[number_layers-2].image->previous=image;
        for (j=number_layers-2; j>=0; j--)
          {
            if (j > 0)
              layer_info[j].image->next=layer_info[j-1].image;
            if (j < (number_layers-1))
              layer_info[j].image->previous=layer_info[j+1].image;
            layer_info[j].image->page.x = layer_info[j].offset_x;
            layer_info[j].image->page.y = layer_info[j].offset_y;
            layer_info[j].image->page.width = layer_info[j].width;
            layer_info[j].image->page.height = layer_info[j].height;

            /* Bob says that if we do this, we'll get REAL gray images! */
            if ( image_type == GIMP_GRAY ) {
              QuantizeInfo  qi;
              GetQuantizeInfo(&qi);
              qi.colorspace = GRAYColorspace;
              (void) QuantizeImage( &qi, layer_info[j].image );
            }
          }
      }
#endif
    }

    MagickFreeMemory(layer_info);

#if 0  /* BOGUS: do we need the channels?? */
    while (True)
      {
        /* read in the offset of the next channel */
        info->cp += xcf_read_int32 (info->fp, &offset, 1);

        /* if the offset is 0 then we are at the end
         *  of the channel list.
         */
        if (offset == 0)
          break;

        /* save the current position as it is where the
         *  next channel offset is stored.
         */
        saved_pos = info->cp;

        /* seek to the channel offset */
        xcf_seek_pos (info, offset);

        /* read in the layer */
        channel = xcf_load_channel (info, gimage);
        if (!channel)
          goto error;

        num_successful_elements++;

        /* add the channel to the image if its not the selection */
        if (channel != gimage->selection_mask)
          gimp_image_add_channel (gimage, channel, -1);

        /* restore the saved position so we'll be ready to
         *  read the next offset.
         */
        xcf_seek_pos (info, saved_pos);
      }
#endif
  }

  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r X C F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterXCFImage adds attributes for the XCF image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterXCFImage method is:
%
%      RegisterXCFImage(void)
%
*/
ModuleExport void RegisterXCFImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("XCF");
  entry->decoder=(DecoderHandler) ReadXCFImage;
  entry->magick=(MagickHandler) IsXCF;
  entry->description="GIMP image";
  entry->module="XCF";
  entry->seekable_stream=True;
  (void) RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r X C F I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterXCFImage removes format registrations made by the
%  XCF module from the list of supported formats.
%
%  The format of the UnregisterXCFImage method is:
%
%      UnregisterXCFImage(void)
%
*/
ModuleExport void UnregisterXCFImage(void)
{
  (void) UnregisterMagickInfo("XCF");
}
