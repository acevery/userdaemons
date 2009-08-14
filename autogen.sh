#!/bin/sh
set -e
set -x

aclocal -I m4
autopoint
automake --add-missing --copy
autoconf
./configure --enable-maintainer-mode $*
