/*
  Copyright (C) 2003 GraphicsMagick Group
  Copyright (C) 2002 ImageMagick Studio
 
  This program is covered by multiple licenses, which are described in
  Copyright.txt. You should have received a copy of Copyright.txt with this
  package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
 
  Windows NT Utility Methods for ImageMagick.
*/
#ifndef _MAGICK_NTBASE_H
#define _MAGICK_NTBASE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/delegate.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <winuser.h>
#include <wingdi.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#if defined(_DEBUG)
#include <crtdbg.h>
#endif

/*
  Define declarations.
*/
#define PROT_READ  1
#define PROT_WRITE  2
#define PROT_READWRITE  3
#define MAP_SHARED  1
#define MAP_PRIVATE  2
#define F_OK 0
#define R_OK 4
#define W_OK 2
#define RW_OK 6
#define HAVE_VSNPRINTF 1
#define HAVE_TEMPNAM 1
#define HAVE_RAISE 1

// Define to support memory mapping files for improved performance
#define HAVE_MMAP 1

// Use Visual C++ C inline method extension to improve performance
#define inline __inline

#if !defined(chsize)
# if defined(__BORLANDC__)
#   define chsize(file,length) chsize(file,length)
# else
#   define chsize(file,length) _chsize(file,length)
# endif
#endif

#if !defined(tempnam)
# define tempnam _tempnam
#endif
#define vsnprintf _vsnprintf 
#if defined(_MT) && defined(WIN32)
#define SAFE_GLOBAL __declspec(thread)
#else
#define SAFE_GLOBAL
#endif

/*
  With Visual C++, Popen and pclose are available via _popen and _pclose.
  These are documented to work for console applications only.
*/
#define HAVE_POPEN 1
#define HAVE__POPEN 1
#define popen  _popen
#define HAVE_PCLOSE 1
#define HAVE__PCLOSE 1
#define pclose  _pclose
/*
  Typedef declarations.
*/
typedef UINT (CALLBACK *LPFNDLLFUNC1)(DWORD,UINT);

#if !defined(XS_VERSION)
struct dirent
{
  char
    d_name[2048];

  int
    d_namlen;
};

/* ssize_t is the type returned by _read and _write */
typedef int ssize_t;

typedef struct _DIR
{
  HANDLE
    hSearch;

  WIN32_FIND_DATA
    Win32FindData;

  BOOL
    firsttime;

  struct dirent
    file_info;
} DIR;

#endif


/*
  NT utilities routines.
*/
extern MagickExport char
  *NTGetLastError(void);

extern MagickExport int
  Exit(int),
  IsWindows95(),
  NTGhostscriptDLL(char *path, int path_length),
  NTGhostscriptEXE(char *path, int path_length),
  NTGhostscriptFonts(char *path, int path_length),
  NTGhostscriptLoadDLL(void),
  NTGhostscriptUnLoadDLL(void),
  NTSystemComman(const char *);

#if defined(MAGICK_IMPLEMENTATION)
extern MagickExport const GhostscriptVectors
  *NTGhostscriptDLLVectors( void );
#endif

#if !defined(XS_VERSION)
extern MagickExport DIR
  *opendir(char *);

extern MagickExport double
  NTElapsedTime(void),
  NTUserTime(void);

extern MagickExport int
#if !defined(HasLTDL)
  lt_dlclose(void *),
  lt_dlexit(void),
  lt_dlinit(void),
  lt_dlsetsearchpath(const char *),
#endif /* !HasLTDL */
  munmap(void *,size_t);

extern MagickExport long
  telldir(DIR *);

extern MagickExport struct dirent
  *readdir(DIR *);

#if !defined(HasLTDL)
extern MagickExport const char
  *lt_dlerror(void);
#endif
  
extern MagickExport unsigned char
  *NTResourceToBlob(const char *);

extern MagickExport unsigned int
  NTGetExecutionPath(char *);

extern MagickExport void
  closedir(DIR *),
  *mmap(char *,size_t,int,int,int,off_t),
  NTErrorHandler(const ExceptionType,const char *,const char *),
  NTWarningHandler(const ExceptionType,const char *,const char *),
  seekdir(DIR *,long)
#if !defined(HasLTDL)
  ,
  *lt_dlopen(const char *),
  *lt_dlsym(void *, const char *)
#endif /* !HasLTDL */
  ;

#endif /* !XS_VERSION */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* !C++ */

#endif /* !_MAGICK_NTBASE_H */
