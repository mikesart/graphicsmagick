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

OUTFILE=TileAnnotate_out.miff
ANNOTATE_CMD_FILE=annotate_cmds_out.txt
echo 'gravity North text 0,20 "Magick"' > ${ANNOTATE_CMD_FILE}
rm -f ${OUTFILE}
${GM} convert ${CONVERT_FLAGS} ${MODEL_MIFF} -fill gold -pointsize 14 \
	-font ${GENERIC_TTF} \
	-draw @${ANNOTATE_CMD_FILE} \
	-label Annotate ${OUTFILE}
