;
; Copyright (C) 2003-2012 GraphicsMagick Group
;
; This program is covered by multiple licenses, which are described in
; Copyright.txt. You should have received a copy of Copyright.txt with this
; package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
;

; #pragma option -v+
; #pragma verboselevel 4
; #define DEBUG 1

#define  public MagickStaticPackage 1
#define  public QuantumDepth "8"
#include "inc\body.isx"

#ifdef Debug
  #expr SaveToFile(AddBackslash(SourcePath) + "gm-win32-Q8-static.isp")
#endif
