# Copyright (C) 1991,92,93,94,95,96,2001 Free Software Foundation, Inc.
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
#	Specific makefile for stdio.
#
subdir	:= stdio

headers	:= stdio.h linewrap.h

routines	:=							      \
	feof ferror clearerr fileno					      \
	newstream fopen freopen fdopen fopncook fmemopen		      \
	setbuf setvbuf setbuffer setlinebuf				      \
	fseek ftell rewind fgetpos fsetpos				      \
	fread fwrite							      \
	ungetc								      \
	fgetc getc getchar						      \
	fputc putc putchar						      \
	fgets gets fputs puts						      \
	getdelim							      \
	fclose fcloseall fflush						      \
	vdprintf vsnprintf vsprintf vasprintf vsscanf vscanf		      \
	memstream obstream linewrap					      \
	__fbufsize __freading __fwriting __freadable __fwritable __flbf	      \
	__fpurge __fpending __fsetlocking				      \
	internals sysd-stdio pipestream stdio_init libc_fatal
aux	:= defs glue


include ../Rules
