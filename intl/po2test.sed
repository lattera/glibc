# po2test.sed - Convert Uniforum style .po file to C code for testing.
# Copyright (C) 2000,2003 Free Software Foundation, Inc.
# Ulrich Drepper <drepper@cygnus.com>, 2000.
#
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
#

#
# We copy the original message as a comment into the .msg file.  But enclose
# them with INPUT ( ).
#
s/^msgid[ 	]*"\(.*\)"/INPUT ("\1")/
# Clear flag from last substitution and jump if matching
tb

#
# Copy the translations as well and enclose them with OUTPUT ( ).
#
s/^msgstr[ 	]*"\(.*\)"/OUTPUT ("\1")/
# Clear flag from last substitution and jump if matching
tb

d

:b
# Append the next line.
$!N
# Check whether second part is a continuation line.  If so, before printing
# insert '\'.
s/\(.*\)")\(\n\)"\(.*\)"/\1\\\2\3")/
P
ta
# No, go to the top and process it. Note that `D' includes a jump to the start!!
D
# Yes, we found a continuation line.
:a
# We cannot use the sed command `D' here
s/[^\n]*\n//
# Clear the substitution flag and do the next line.
tb
