#!/bin/sh
# Copyright (C) 2004-2010 GraphicsMagick Group
. ${srcdir}/tests/common.shi
${MEMCHECK} ./constitute -storagetype long ${SRCDIR}/input_truecolor.miff cmy
