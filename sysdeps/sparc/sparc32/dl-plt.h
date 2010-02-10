/* PLT fixups.  Sparc 32-bit version.
   Copyright (C) 1996-2003, 2004, 2005, 2006, 2007, 2010
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Some SPARC opcodes we need to use for self-modifying code.  */
#define OPCODE_NOP	0x01000000 /* nop */
#define OPCODE_CALL	0x40000000 /* call ?; add PC-rel word address */
#define OPCODE_SETHI_G1	0x03000000 /* sethi ?, %g1; add value>>10 */
#define OPCODE_JMP_G1	0x81c06000 /* jmp %g1+?; add lo 10 bits of value */
#define OPCODE_SAVE_SP	0x9de3bfa8 /* save %sp, -(16+6)*4, %sp */
#define OPCODE_BA	0x30800000 /* b,a ?; add PC-rel word address */

static inline __attribute__ ((always_inline)) Elf32_Addr
sparc_fixup_plt (const Elf32_Rela *reloc, Elf32_Addr *reloc_addr,
		 Elf32_Addr value, int t, int do_flush)
{
  Elf32_Sword disp = value - (Elf32_Addr) reloc_addr;

  if (0 && disp >= -0x800000 && disp < 0x800000)
    {
      /* Don't need to worry about thread safety. We're writing just one
	 instruction.  */

      reloc_addr[0] = OPCODE_BA | ((disp >> 2) & 0x3fffff);
      if (do_flush)
	__asm __volatile ("flush %0" : : "r"(reloc_addr));
    }
  else
    {
      /* For thread safety, write the instructions from the bottom and
	 flush before we overwrite the critical "b,a".  This of course
	 need not be done during bootstrapping, since there are no threads.
	 But we also can't tell if we _can_ use flush, so don't. */

      reloc_addr += t;
      reloc_addr[1] = OPCODE_JMP_G1 | (value & 0x3ff);
      if (do_flush)
	__asm __volatile ("flush %0+4" : : "r"(reloc_addr));

      reloc_addr[0] = OPCODE_SETHI_G1 | (value >> 10);
      if (do_flush)
	__asm __volatile ("flush %0" : : "r"(reloc_addr));
    }

  return value;
}
