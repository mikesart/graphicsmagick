#!/bin/sh
# Copyright (C) 2009 GraphicsMagick Group
#
# This program is covered by multiple licenses, which are described in
# Copyright.txt. You should have received a copy of Copyright.txt with this
# package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
#
#  Test for 'gm' utility.
#

set -e # Exit on any error
. ${srcdir}/utilities/tests/common.sh

OUTFILE=TileRecolor_out.miff
MATRIXFILE=recolor_matrix_out.txt
echo '0.9 0 0, 0 0.9 0, 0 0 1.2' > ${MATRIXFILE}
rm -f ${OUTFILE}
${GM} convert ${CONVERT_FLAGS} ${MODEL_MIFF} -recolor @${MATRIXFILE} -label Recolor ${OUTFILE}
rm -f ${MATRIXFILE}