# Copyright (C) 1995 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Sub-makefile for sysvipc portion of the library.
#
subdir	:= sysvipc

headers	:= sys/ipc.h sys/msg.h sys/sem.h sys/shm.h \
	   sys/ipc_buf.h sys/msq_buf.h sys/sem_buf.h sys/shm_buf.h

routines := msgsnd msgrcv msgget msgctl \
	    semop semget semctl \
	    shmat shmdt shmget shmctl

include ../Rules
