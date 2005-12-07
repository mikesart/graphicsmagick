/* lt__dirent.h -- internal directory entry scanning interface
   Copyright (C) 2001, 2004 Free Software Foundation, Inc.
   Originally by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>

   NOTE: The canonical source of this file is maintained with the
   GNU Libtool package.  Report bugs to bug-libtool@gnu.org.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

As a special exception to the GNU Lesser General Public License,
if you distribute this file as part of a program or library that
is built using GNU libtool, you may include it under the same
distribution terms that you use for the rest of that program.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301  USA

*/

#if !defined(LT__DIRENT_H)
#define LT__DIRENT_H 1

#if defined(HAVE_CONFIG_H)
#  if defined(LT_CONFIG_H)
#    include LT_CONFIG_H
#  else
#    include <config.h>
#  endif
#endif

#include "lt_system.h"

#if defined(HAVE_CLOSEDIR) && defined(HAVE_OPENDIR) && defined(HAVE_READDIR) && defined(HAVE_DIRENT_H)
/* We have a fully operational dirent subsystem.  */
#  include <dirent.h>
#  define D_NAMLEN(dirent) (strlen((dirent)->d_name))

#elif !defined(__WINDOWS__)
/* We are not on windows, so we can get the same functionality from the
   `direct' API.  */
#  define dirent direct
#  define D_NAMLEN(dirent) ((dirent)->d_namlen)
#  if defined(HAVE_SYS_NDIR_H)
#    include <sys/ndir.h>
#  endif
#  if defined(HAVE_SYS_DIR_H)
#    include <sys/dir.h>
#  endif
#  if defined(HAVE_NDIR_H)
#    include <ndir.h>
#  endif

#else  /* __WINDOWS__ */
/* Use some wrapper code to emulate dirent on windows..  */
#  define WINDOWS_DIRENT_EMULATION 1

#  include <windows.h>

#  define D_NAMLEN(dirent)	(strlen((dirent)->d_name))
#  define dirent		lt__dirent
#  define DIR			lt__DIR
#  define opendir		lt__opendir
#  define readdir		lt__readdir
#  define closedir		lt__closedir

LT_BEGIN_C_DECLS

struct dirent
{
  char d_name[LT_FILENAME_MAX];
  int  d_namlen;
};

typedef struct
{
  HANDLE hSearch;
  WIN32_FIND_DATA Win32FindData;
  BOOL firsttime;
  struct dirent file_info;
} DIR;


LT_SCOPE DIR *		opendir		(const char *path);
LT_SCOPE struct dirent *readdir		(DIR *entry);
LT_SCOPE void		closedir	(DIR *entry);

LT_END_C_DECLS

#endif /*!defined(__WINDOWS__)*/

#endif /*!defined(LT__DIRENT_H)*/
