/* Special .init and .fini section support.
Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* This file is compiled into assembly code which is then surrounded by the
   lines `cat > crtcommon.tmp <<\EOF_common' and `EOF_common' and thus
   becomes a shell script which creates three files of assembly code.

   * The first file is crti.s-new; this puts a function prologue at the
   beginning of the .init and .fini sections and defines global symbols for
   those addresses, so they can be called as functions.

   * The second file is crtn.s-new; this puts the corresponding function
   epilogues in the .init and .fini sections.

   * The third file is crtcommon.tmp, which is whatever miscellaneous cruft
   the compiler generated at the end; it should be appended to both crti.s-new
   and crtn.s-new.  */

#include <stdlib.h>

/* These declarations make the functions go in the right sections when
   we define them below.  GCC syntax does not allow the attribute
   specifications to be in the function definitions themselves.  */
void _init (void) __attribute__ ((section (".init")));
void _fini (void) __attribute__ ((section (".fini")));

/* End the here document containing the initial common code.
   Then move the output file crtcommon.tmp to crti.s-new and crtn.s-new.  */
asm ("\nEOF_common\n\
mv -f crtcommon.tmp crti.s-new\n\
cp -f crti.s-new crtn.s-new");

/* Append the .init prologue to crti.s-new.  */
asm ("cat >> crti.s-new <<\\EOF.crti.init");
void
_init (void)
{
  (void) &_init;		/* Don't optimize out the function! */
  /* End the here document containing the .init prologue code.
     Then fetch the .section directive just written and append that
     to crtn.s-new, followed by the function epilogue.  */
  asm ("\nEOF.crti.init
\n\
	fgrep .init crti.s-new >>crtn.s-new\n\
	cat >> crtn.s-new <<\\EOF.crtn.init");
}

/* End the here document containing the .init epilogue code.
   Then append the .fini prologue to crti.s-new.  */
asm ("\nEOF.crtn.init\
\n\
cat >> crti.s-new <<\\EOF.crti.fini");

void
_fini (void)
{
  (void) &_fini;		/* Don't optimize out the function! */
  /* End the here document containing the .fini prologue code.
     Then fetch the .section directive just written and append that
     to crtn.s-new, followed by the function epilogue.  */
  asm ("\nEOF.crti.fini\
\n\
	fgrep .fini crti.s-new >>crtn.s-new\n\
	cat >> crtn.s-new <<\\EOF.crtn.fini");
}

/* End the here document containing the .fini epilogue code.
   Finally, put the remainder of the generated assembly into crtcommon.tmp.  */
asm ("\nEOF.crtn.fini\
\n\
cat > crtcommon.tmp <<\\EOF_common");
