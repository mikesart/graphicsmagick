#!/bin/sh
# Copyright (C) 2004-2010 GraphicsMagick Group
. ${srcdir}/tests/common.shi
${MEMCHECK} ./rwfile  -filespec 'out_pallette_rle_%d' -compress rle ${SRCDIR}/input_pallette.miff PDF
