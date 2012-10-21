#! /bin/sh
# Test whether all cancelable functions are cancelable.
# Copyright (C) 2002-2012 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

NM="$1"; shift
while [ $# -gt 0 ]; do
  ( $NM -P $1; echo 'end[end]:' ) | gawk ' BEGIN {
C["accept"]=1
C["close"]=1
C["connect"]=1
C["creat"]=1
C["fcntl"]=1
C["fdatasync"]=1
C["fsync"]=1
C["msgrcv"]=1
C["msgsnd"]=1
C["msync"]=1
C["nanosleep"]=1
C["open"]=1
C["open64"]=1
C["pause"]=1
C["poll"]=1
C["pread"]=1
C["pread64"]=1
C["pselect"]=1
C["pwrite"]=1
C["pwrite64"]=1
C["read"]=1
C["readv"]=1
C["recv"]=1
C["recvfrom"]=1
C["recvmsg"]=1
C["select"]=1
C["send"]=1
C["sendmsg"]=1
C["sendto"]=1
C["sigpause"]=1
C["sigsuspend"]=1
C["sigwait"]=1
C["sigwaitinfo"]=1
C["system"]=1
C["tcdrain"]=1
C["wait"]=1
C["waitid"]=1
C["waitpid"]=1
C["write"]=1
C["writev"]=1
C["__xpg_sigpause"]=1
}
/:$/ {
  if (seen)
    {
      if (!seen_enable || !seen_disable)
	{
	  printf "in '$1'(%s) %s'\''s cancellation missing\n", object, seen
	  ret = 1
	}
    }
  seen=""
  seen_enable=""
  seen_disable=""
  object=gensub(/^.*\[(.*)\]:$/,"\\1","",$0)
  next
}
{
  if (C[$1] && $2 ~ /^[TW]$/)
    seen=$1
  else if ($1 ~ /^([.]|)__(libc|pthread)_enable_asynccancel$/ && $2 == "U")
    seen_enable=1
  else if ($1 ~ /^([.]|)__(libc|pthread)_disable_asynccancel$/ && $2 == "U")
    seen_disable=1
}
END {
  exit ret
}' || exit
  shift
done
