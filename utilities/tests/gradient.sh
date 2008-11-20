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

OUTFILE=TileGradient_out.miff
rm -f ${OUTFILE}
${GM} convert ${CONVERT_FLAGS} -size 130x194 gradient:'#20a0ff-#ffff00' -label Gradient ${OUTFILE}
