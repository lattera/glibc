#! /bin/bash
# Copyright (C) 2003 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.
#
# The list of data types has been created with
# cat <<EOF |
# #include <sys/types.h>
# #include <unistd.h>
# #include <sys/resource.h>
# #include <sys/stat.h>
# EOF
# gcc -D_GNU_SOURCE -E - |
# egrep '^typedef.*;$' |
# sed 's/^typedef[[:space:]]*//;s/\([[:space:]]\{1,\}__attribute__.*\);/;/;s/.*[[:space:]]\([*]\|\)\(.*\);/\2/' |
# egrep -v '^_' |
# sort -u
#
data=$1
shift
cxx="$*"
while read t; do
  echo -n "$t:"
  $cxx -S -xc++ -o - -D_GNU_SOURCE <(cat <<EOF
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
void foo ($t) { }
EOF
) |
  sed 's/[[:space:]]*[.]globa\?l[[:space:]]*_Z3foo\([_[:alnum:]]*\).*/\1/p;d'
done <<EOF |
blkcnt64_t
blkcnt_t
blksize_t
caddr_t
clockid_t
clock_t
daddr_t
dev_t
fd_mask
fsblkcnt64_t
fsblkcnt_t
fsfilcnt64_t
fsfilcnt_t
fsid_t
gid_t
id_t
ino64_t
ino_t
int16_t
int32_t
int64_t
int8_t
intptr_t
key_t
loff_t
mode_t
nlink_t
off64_t
off_t
pid_t
pthread_key_t
pthread_once_t
pthread_spinlock_t
pthread_t
quad_t
register_t
rlim64_t
rlim_t
sigset_t
size_t
socklen_t
ssize_t
suseconds_t
time_t
u_char
uid_t
uint
u_int
u_int16_t
u_int32_t
u_int64_t
u_int8_t
ulong
u_long
u_quad_t
useconds_t
ushort
u_short
EOF
diff -N -U0 $data -
