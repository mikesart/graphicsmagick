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
%                            V   V  M   M  SSSSS                              %
%                            V   V  MM MM  SS                                 %
%                            V   V  M M M   SSS                               %
%                             V V   M   M     SS                              %
%                              V    M   M  SSSSS                              %
%                                                                             %
%                                                                             %
%                   VMS Utility Methods for GraphicsMagick                    %
%                                                                             %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                October 1994                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The directory methods are strongly based on similar methods written
%  by Rich Salz.
%
*/

#if defined(vms)
/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/utility.h"
#include "magick/vms.h"

#if !defined(_AXP_) && (!defined(__VMS_VER) || (__VMS_VER < 70000000))
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   c l o s e d i r                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method closedir closes the named directory stream and frees the DIR
%  structure.
%
%  The format of the closedir method is:
%
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
void closedir(DIR *directory)
{
  assert(directory != (DIR *) NULL);
  MagickFreeMemory(directory->pattern);
  MagickFreeMemory(directory);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   o p e n d i r                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method opendir opens the directory named by filename and associates
%  a directory stream with it.
%
%  The format of the opendir method is:
%
%      opendir(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
DIR *opendir(char *name)
{
  DIR
    *directory;

  /*
    Allocate memory for handle and the pattern.
  */
  directory=MagickAllocateMemory(DIR *,sizeof(DIR));
  if (directory == (DIR *) NULL)
    {
      errno=ENOMEM;
      return((DIR *) NULL);
    }
  if (strcmp(".",name) == 0)
    name="";
  directory->pattern=MagickAllocateMemory(char *,strlen(name)+sizeof("*.*")+1);
  if (directory->pattern == (char *) NULL)
    {
      MagickFreeMemory(directory);
      errno=ENOMEM;
      return(NULL);
    }
  /*
    Initialize descriptor.
  */
  FormatString(directory->pattern,"%.1024s*.*",name);
  directory->context=0;
  directory->pat.dsc$a_pointer=directory->pattern;
  directory->pat.dsc$w_length=strlen(directory->pattern);
  directory->pat.dsc$b_dtype=DSC$K_DTYPE_T;
  directory->pat.dsc$b_class=DSC$K_CLASS_S;
  return(directory);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   r e a d d i r                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method readdir returns a pointer to a structure representing the
%  directory entry at the current position in the directory stream to
%  which entry refers.
%
%  The format of the readdir
%
%      readdir(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
struct dirent *readdir(DIR *directory)
{
  char
    buffer[sizeof(directory->entry.d_name)];

  int
    status;

  register char
    *p;

  register int
    i;

  struct dsc$descriptor_s
    result;

  /*
    Initialize the result descriptor.
  */
  result.dsc$a_pointer=buffer;
  result.dsc$w_length=sizeof(buffer)-2;
  result.dsc$b_dtype=DSC$K_DTYPE_T;
  result.dsc$b_class=DSC$K_CLASS_S;
  status=lib$find_file(&directory->pat,&result,&directory->context);
  if ((status == RMS$_NMF) || (directory->context == 0L))
    return((struct dirent *) NULL);
  /*
    Lowercase all filenames.
  */
  buffer[sizeof(buffer)-1]='\0';
  for (p=buffer; *p; p++)
    if (isupper(*p))
      *p=tolower(*p);
  /*
    Skip any directory component and just copy the name.
  */
  p=buffer;
  while (!isspace(*p))
    p++;
  *p='\0';
  p=strchr(buffer,']');
  if (p)
    (void) strlcpy(directory->entry.d_name,p+1,MaxTextExtent);
  else
    (void) strlcpy(directory->entry.d_name,buffer,MaxTextExtent);
  directory->entry.d_namlen=strlen(directory->entry.d_name);
  return(&directory->entry);
}
#endif /* !defined(_AXP_) && (!defined(__VMS_VER) || (__VMS_VER < 70000000)) */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M a g i c k C o n f l i c t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method VMSIsMagickConflict returns true if the image format conflicts with
%  with a logical drive (.e.g. SYS$SCRATCH:).
%
%  Contributed by Forrest Cahoon (forrest@wiredaemons.com)
%
%  The format of the VMSIsMagickConflict method is:
%
%      status=VMSIsMagickConflict(const char *magick)
%
%  A description of each parameter follows:
%
%    o status: Method VMSIsMagickConflict returns true if the image format
%      conflicts with a logical drive.
%
%    o magick: Specifies the image format.
%
%
*/
MagickExport unsigned int VMSIsMagickConflict(const char *magick)
{
  ile3
    item_list[2];

  int
    device_class,
    status;

  struct dsc$descriptor_s
    device;

  assert(magick != (char *) NULL);
  device.dsc$w_length=strlen(magick);
  device.dsc$a_pointer=(char *) magick;
  device.dsc$b_class=DSC$K_CLASS_S;
  device.dsc$b_dtype=DSC$K_DTYPE_T;
  item_list[0].ile3$w_length=sizeof(device_class);
  item_list[0].ile3$w_code=DVI$_DEVCLASS;
  item_list[0].ile3$ps_bufaddr=&device_class;
  item_list[0].ile3$ps_retlen_addr=NULL;
  (void) memset(&item_list[1],0,sizeof(item_list[1]));
  status=sys$getdviw(0,0,&device,&item_list,0,0,0,0);
  if ((status == SS$_NONLOCAL) ||
      ((status & 0x01) && (device_class & (DC$_DISK | DC$_TAPE))))
    return(True);
  return(False);
}
#endif /* defined(vms) */
