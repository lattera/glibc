# Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

#
#	Specific makefile for libio.
#
subdir	:= libio

headers	:= stdio.h libio.h _G_config.h

routines	:=							      \
	filedoalloc iofclose iofdopen iofflush iofgetpos iofgets iofopen      \
	iofopncook iofprintf iofputs iofread iofsetpos ioftell		      \
	iofwrite iogetdelim iogetline iogets iopadn iopopen ioputs	      \
	ioseekoff ioseekpos iosetbuffer iosetvbuf iosprintf ioungetc	      \
	iovsprintf iovsscanf						      \
									      \
	clearerr feof ferror fgetc fileno fputc freopen fseek getc getchar    \
	memstream pclose putc putchar rewind setbuf setlinebuf vasprintf      \
	vdprintf vscanf vsnprintf obprintf fcloseall			      \
									      \
	libc_fatal

all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig

CPPFLAGS-.o += -DIO_DEBUG

ifneq (,$(filter %REENTRANT, $(defines)))
routines += clearerr_u feof_u ferror_u fputc_u getc_u getchar_u		      \
	    iofflush_u putc_u putchar_u peekc

CPPFLAGS += -D_IO_MTSAFE_IO
endif

aux	:=								      \
	fileops genops stdfiles stdio strops

distribute := iolibio.h libioP.h strfile.h Banner

include ../Rules
