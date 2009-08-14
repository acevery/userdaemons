# Process this file with autoconf to produce a configure script.
# vim:set et ts=4
#
# userdaemons -- run daemon as uid of users set in /etc/userdaemons.conf
# and set the cpu_rt_runtime then.
#
# Copyright (c) 2008-2009 Yu Yuwei <acevery@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

# if not 1, append datestamp to the version number.
m4_define([package_name], [userdaemons])
m4_define([released], [0])
m4_define([major_version], [1])
m4_define([minor_version], [2])
m4_define([micro_version], [0])
m4_define(maybe_datestamp,
    m4_esyscmd([if test x]released[ != x1; then date +.%Y%m%d | tr -d '\n\r'; fi]))

m4_define([version],
    major_version.minor_version.micro_version[]maybe_datestamp)

AC_INIT([package_name], [version], [http://code.google.com/p/userdaemons], [package_name])
AM_INIT_AUTOMAKE([1.10])
AM_MAINTAINER_MODE
# AC_GNU_SOURCE

# AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC

# OUTPUT files
AC_CONFIG_FILES([Makefile
    src/Makefile
    conf/Makefile
    m4/Makefile
    ]
)

AC_OUTPUT
# EOF
