#!/bin/sh
# Copyright (C) 2004-2010 GraphicsMagick Group
. ${srcdir}/tests/common.shi
${MEMCHECK} ./rwfile -filespec 'out_truecolor16_%d' ${SRCDIR}/input_truecolor16.dpx CIN
