/* Special .init and .fini section support.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Library General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file.  (The Library General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   The GNU C Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

/* The basic layout of note sections is specified by the ELF format.  */

#define ELF_NOTE_BEGIN(sectname, sectflags, type, name)			      \
	.section sectname, sectflags;					      \
	.align ALIGNARG(2);	/* Notes are 4-byte aligned.  */	      \
	.long 1f - 0f;		/* 32-bit word: length of name field */	      \
	.long 3f - 2f;		/* 32-bit word: length of desc field */	      \
	.long (type);		/* 32-bit word: vendor-defined type field */  \
0:	.asciz name;		/* null-terminated string, any length: name */\
1:	.align ALIGNARG(2);	/* Name data padded to 4-byte alignment.  */  \
2:	/* Here follows the "note descriptor" data, whose format	      \
	   is not specified by ELF.  The vendor name and type field	      \
	   indicate what sort of data is found here.  */

#define ELF_NOTE_END							      \
3:	.align ALIGNARG(2)	/* Pad to 4-byte align the next note.  */


/* The linker (GNU ld 2.8 and later) recognize an allocated section whose
   name begins with `.note' and creates a PT_NOTE program header entry
   pointing at it.

   Such a program header is the canonical way (at least in the free OS
   community) to identify the OS environment ABI that the executable was
   created for.  The ELF note information identifies a particular OS or
   coordinated development effort within which the ELF header's e_machine
   value plus (for dynamically linked programs) the PT_INTERP dynamic
   linker name and DT_NEEDED shared library names fully identify the
   runtime environment required by an executable.

   The GNU project and cooperating development efforts (including the Linux
   community) use a vendor name string of "GNU", and a note type field with
   value 1 for a note descriptor that indicates ABI requirements.  */

#include <abi-tag.h>		/* OS-specific ABI tag value */

ELF_NOTE_BEGIN(".note.ABI-tag", "a", 1, "GNU")
	.long ABI_TAG
ELF_NOTE_END
