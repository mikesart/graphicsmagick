#!/bin/sh
# Copyright (C) 2004-2010 GraphicsMagick Group
. ${srcdir}/tests/common.shi
${MEMCHECK} ./rwfile  -filespec 'out_pallette_zip_%d' -compress zip ${SRCDIR}/input_pallette.miff MIFF
