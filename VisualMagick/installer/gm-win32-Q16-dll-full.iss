;
; Copyright (C) 2003-2012 GraphicsMagick Group
;
; This program is covered by multiple licenses, which are described in
; Copyright.txt. You should have received a copy of Copyright.txt with this
; package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
;

; This builds an installer for a Q16 DLL build which includes programs
; and DLLs based on the MSVC-propritary COM and ATL class libraries.

; #pragma option -v+
; #pragma verboselevel 4
; #define DEBUG 1

#define  public MagickDynamicPackage 1
#define  public QuantumDepth "16"
#define  public MSVCProprietary 1
#define  public InstallPerlMagick 1
#include "inc\body.isx"

#ifdef Debug
  #expr SaveToFile(AddBackslash(SourcePath) + "gm-win32-Q16-dll-full.isp")
#endif
