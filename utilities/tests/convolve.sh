#!/bin/sh
# Copyright (C) 2004 GraphicsMagick Group
#
# This program is covered by multiple licenses, which are described in
# Copyright.txt. You should have received a copy of Copyright.txt with this
# package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
#
#  Test for 'gm' utility.
#

set -e # Exit on any error
. ${srcdir}/utilities/tests/common.sh

OUTFILE=TileConvolve_out.miff
rm -f ${OUTFILE}
# 1,1,1,
# 1,4,1,
# 1,1,1
${GM} convert ${CONVERT_FLAGS} ${MODEL_MIFF} -convolve 1,1,1,1,4,1,1,1,1 -label Convolve ${OUTFILE}
