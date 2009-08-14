# vim:set noet ts=4
#
# userdaemons -- run daemon as uid of users set in /etc/userdaemons.conf
# and set the cpu_rt_runtime then.
#
# Copyright (c) 2009- Yu Yuwei <acevery@gmail.com>
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

sbin_PROGRAMS = userdaemon \
	userdaemons \
	$(NULL)
userdaemon_SOURCES = userdaemon.c \
	$(NULL)
userdaemons_SOURCES = userdaemons.c \
	$(NULL)

LIBS += -lpthread

MAINTAINERCLEANFILES = \
	Makefile.in \
	$(NULL)

# EOF
