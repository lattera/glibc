# Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
#	Specific makefile for stdio.
#
subdir	:= stdio

headers	:= stdio.h stdio_lim.h printf.h

routines	:=							      \
	ctermid cuserid							      \
	feof ferror clearerr fileno					      \
	newstream fopen freopen fdopen fopncook fmemopen		      \
	setbuf setvbuf setbuffer setlinebuf				      \
	fseek ftell rewind fgetpos fsetpos				      \
	vfprintf vprintf printf_fp reg-printf printf-prs _itoa		      \
	vsnprintf vsprintf vasprintf	      				      \
	fprintf printf snprintf sprintf asprintf			      \
	dprintf vdprintf						      \
	vfscanf vscanf vsscanf						      \
	fscanf scanf sscanf						      \
	fread fwrite							      \
	ungetc								      \
	fgetc getc getchar getw						      \
	fputc putc putchar putw						      \
	fgets gets fputs puts						      \
	getdelim getline						      \
	perror psignal							      \
	tmpfile tmpnam tempnam tempname					      \
	fclose fflush							      \
	remove rename							      \
	memstream obstream						      \
	internals sysd-stdio pipestream stdio_init libc_fatal

# Several mpn functions from GNU MP are used by the printf_fp function.
mpn-routines := add_1 add_n addmul_1 cmp divmod divmod_1 udiv_qrnnd \
		lshift rshift mod_1 mul mul_1 mul_n sub_n submul_1 
mpn-headers = longlong.h gmp.h gmp-impl.h gmp-mparam.h asm-syntax.h

routines := $(strip $(routines) $(mpn-routines))	\
	    dbl2mpn ldbl2mpn				\
	    mpn2flt mpn2dbl mpn2ldbl
aux	:= errlist siglist defs glue mp_clz_tab fpioconst 
distribute = $(mpn-headers) gen-mpn-copy _itoa.h fpioconst.h

tests := tst-printf tstscanf test_rdwr test-popen tstgetln test-fseek \
	 temptest tst-fileno test-fwrite \
	 xbug errnobug \
	 bug1 bug2 bug3 bug4 bug5 bug6 bug7


include ../Rules


ifdef gmp-srcdir

gmp-srcdir := $(firstword $(filter-out ..//%,$(..)$(gmp-srcdir) $(gmp-srcdir)))

# Copy the mpn source files we use from the GNU MP source directory.
# `gmp-srcdir' is set by doing `configure --with-gmp=DIR'.
# (Do not try this at home.  You need an as yet unreleased version of GNU MP.)

mpn-sysdep := $(addsuffix .c,$(mpn-routines)) \
	      $(addsuffix .S,$(mpn-routines)) \
	      $(addsuffix .s,$(mpn-routines)) gmp-mparam.h asm-syntax.h

mpn-try := $(addprefix $(gmp-srcdir)/mpn/*/,$(mpn-sysdep))
mpn-found := $(wildcard $(mpn-try))
mpn-found := $(filter-out $(patsubst %.S,%.s,$(filter %.s,$(mpn-found))),\
			  $(mpn-found))

include mpn-copy.mk
%.mk: gen-%; sh $< > $@

mpn-copy-1 := $(patsubst $(gmp-srcdir)/mpn/%,$(sysdep_dir)/%,$(mpn-found))
mpn-copy-sysdep := $(mpn-copy-sysdep) $(mpn-copy-1)
$(mpn-copy-1): $(sysdep_dir)/%: $(ignore gmp2glibc.sed) $(gmp-srcdir)/mpn/%
	$(gmp2glibc)

mpn-stuff = $(mpn-copy-sysdep) $(mpn-copy)

# chmod so I don't edit them by mistake.
define gmp2glibc
$(ignore sed -f $^ > $@-tmp)
cp $< $@-tmp
chmod a-w $@-tmp
mv -f $@-tmp $@
endef

mpn-copy = $(filter-out $(mpn-sysdep),$(mpn-headers) mp_clz_tab.c)
$(mpn-copy): %: $(ignore gmp2glibc.sed) $(gmp-srcdir)/%; $(gmp2glibc)

.PHONY: copy-mpn clean-mpn
copy-mpn: $(mpn-stuff)
clean-mpn:
	rm -f $(mpn-stuff)

endif
