#! /bin/sh -f
#
# Copyright (C) 1998, 1999 Free Software Foundation, Inc.
# This file is part of the GNU C Library and contains tests for
# the rpmatch(3)-implementation.
# contributed by Jochen Hein <jochen.hein@delphi.central.de>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

common_objpfx=$1
IFS="&"
rc=0
while read locale string result dummy; do
    if [ "$locale" != "#" ]; then
	LOCPATH=${common_objpfx}localedata \
	${common_objpfx}elf/ld.so --library-path $common_objpfx \
	${common_objpfx}localedata/tst-rpmatch $locale $string $result \
	|| exit 1
    fi
done <<EOF
#& These are the tests for rpmatch in glibc.  Each line contains one test,
#& comments start with #& in the first column.  The fields are seperated
#& by paragraph signs and contain: the locale, the string, the expected
#& return value of rpmatch(3).  If the test fails, test-rpmatch prints
#& all these informations
C&Yes&1
C&yes&1
C&YES&1
C&YeS&1
C&YEs&1
C&yEs&1
C&yES&1
C&yeS&1
C&No&0
C&no&0
#& Uh, that's nonsense
C&nonsens&0
C&Error&-1
de_DE&Yes&1
de_DE&Ja&1
de_DE&Jammerschade&1
de_DE&dejavu&-1
de_DE&Nein&0
de_DE&Fehler&-1
de_DE&jein&1
EOF

rc=$?
if [ $rc -eq 1 ]; then
    echo "Errors"
fi
exit $rc
