/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.
   Based on work contributed by by Carl Pederson & Martin Schwidefsky.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef dl_machine_h
#define dl_machine_h

#ifdef __tilegx__
#define ELF_MACHINE_NAME "tilegx"
#else
#define ELF_MACHINE_NAME "tilepro"
#endif

#include <sys/param.h>
#include <string.h>
#include <link.h>
#include <bits/wordsize.h>
#include <arch/icache.h>
#include <arch/opcode.h>

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const ElfW(Ehdr) *ehdr)
{
#if defined __tilegx__
  if (ehdr->e_machine != EM_TILEGX)
    return 0;
# if __WORDSIZE == 32
  return (ehdr->e_ident[EI_CLASS] == ELFCLASS32);
# else
  return (ehdr->e_ident[EI_CLASS] == ELFCLASS64);
# endif
#elif defined __tilepro__
  return ehdr->e_machine == EM_TILEPRO;
#else
# error "Unknown tile architecture."
#endif
}


/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */

static inline ElfW(Addr)
elf_machine_dynamic (void)
{
  ElfW(Addr) *got;

#ifdef __tilegx__
  ElfW(Addr) tmp;
  asm( "	{ lnk %0; moveli %1, hw2_last(_GLOBAL_OFFSET_TABLE_ - 1f) }\n"
       "1:	shl16insli %1, %1, hw1(_GLOBAL_OFFSET_TABLE_ - 1b)\n"
       "	shl16insli %1, %1, hw0(_GLOBAL_OFFSET_TABLE_ - 1b)\n"
       "        add %0, %0, %1"
       : "=r" (got), "=r" (tmp));
#else
  asm( "	lnk %0\n"
       "1:	addli %0, %0, lo16(_GLOBAL_OFFSET_TABLE_ - 1b)\n"
       "	auli %0, %0, ha16(_GLOBAL_OFFSET_TABLE_ - 1b)"
       : "=r" (got));
#endif

  return *got;
}


/* Return the run-time load address of the shared object.  */
static inline ElfW(Addr)
elf_machine_load_address (void)
{
  ElfW(Addr) *got;
  ElfW(Addr) dynamic;

#ifdef __tilegx__
  ElfW(Addr) tmp;
  asm( "	lnk %2\n"
       "1:	{\n"
       "	 moveli %0, hw2_last(_GLOBAL_OFFSET_TABLE_ - 1b)\n"
       "	 moveli %1, hw2_last(_DYNAMIC - 1b)\n"
       "	}\n"
       "	{\n"
       "	 shl16insli %0, %0, hw1(_GLOBAL_OFFSET_TABLE_ - 1b)\n"
       "	 shl16insli %1, %1, hw1(_DYNAMIC - 1b)\n"
       "	}\n"
       "	{\n"
       "	 shl16insli %0, %0, hw0(_GLOBAL_OFFSET_TABLE_ - 1b)\n"
       "	 shl16insli %1, %1, hw0(_DYNAMIC - 1b)\n"
       "	}\n"
       "	{\n"
       "	 add %0, %0, %2\n"
       "	 add %1, %1, %2\n"
       "	}"
       : "=r" (got), "=r" (dynamic), "=r" (tmp));
#else
  asm( "	lnk %0\n"
       "1:	{\n"
       "	 addli %0, %0, lo16(_GLOBAL_OFFSET_TABLE_ - 1b)\n"
       "	 addli %1, %0, lo16(_DYNAMIC - 1b)\n"
       "	}\n"
       "	{\n"
       "	 auli %0, %0, ha16(_GLOBAL_OFFSET_TABLE_ - 1b)\n"
       "	 auli %1, %1, ha16(_DYNAMIC - 1b)\n"
       "	}\n"
       : "=r" (got), "=r" (dynamic));
#endif

  return dynamic - *got;
}

/* Flush some range of the instruction cache.  If invoked prior to
   actually setting dl_pagesize, we conservatively use 4KB, which
   is the smallest page size we could plausibly be running with.  */
static inline void
_dl_flush_icache (const void *addr, unsigned long size)
{
  invalidate_icache (addr, size, GLRO(dl_pagesize) ? : 4096);
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int __attribute__ ((unused))
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  ElfW(Addr) *gotplt;
  extern void _dl_runtime_resolve (ElfW(Word));
  extern void _dl_runtime_profile (ElfW(Word));

  if (l->l_info[DT_JMPREL] && lazy)
    {
      gotplt = (ElfW(Addr) *) D_PTR (l, l_info[DT_PLTGOT]);

      /* The GOT entries for functions in the PLT have not yet been filled
         in.  Their initial contents will arrange when called to put in
         registers an offset into the .rel.plt section, and gotplt[0], then
         jump to gotplt[1].  */

      /* Identify this shared object.  */
      gotplt[0] = (ElfW(Addr)) l;

      /* The gotplt[1] entry contains the address of a function which gets
         called to get the address of a so far unresolved function and jump
         to it.  The profiling extension of the dynamic linker allows to
         intercept the calls to collect information.  In this case we don't
         store the address in the GOTPLT so that all future calls also end
         in this function.  */
      if (__builtin_expect (profile, 0))
        {
          gotplt[1] = (ElfW(Addr)) &_dl_runtime_profile;

          if (GLRO(dl_profile) != NULL
              && _dl_name_match_p (GLRO(dl_profile), l))
            /* This is the object we are looking for.  Say that we really
               want profiling and the timers are started.  */
            GL(dl_profile_map) = l;
        }
      else
        /* This function will get called to fix up the GOTPLT entry
           indicated by the offset on the stack, and then jump to the
           resolved address.  */
        gotplt[1] = (ElfW(Addr)) &_dl_runtime_resolve;
    }

  return lazy;
}

#if __WORDSIZE == 32
/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK   0xf8000000UL
#endif

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm (".globl _dl_start");

#ifndef RTLD_START_SPECIAL_INIT
#define RTLD_START_SPECIAL_INIT /* nothing */
#endif

/* Wrap a generic Tilera relocation type. */
#ifdef __tilegx__
#define R_TILE(x) R_TILEGX_##x
#define __R_TILE_TLS(x,c) R_TILEGX_TLS_##x##c
#define _R_TILE_TLS(x,c) __R_TILE_TLS(x,c)
#define R_TILE_TLS(x) _R_TILE_TLS(x,__ELF_NATIVE_CLASS)
#else
#define R_TILE(x) R_TILEPRO_##x
#define R_TILE_TLS(x) R_TILEPRO_TLS_##x##32
#endif

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_COPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_TILE(JMP_SLOT) || (type) == R_TILE_TLS(DTPMOD)		      \
     || (type) == R_TILE_TLS(DTPOFF) || (type) == R_TILE_TLS(TPOFF))	      \
    * ELF_RTYPE_CLASS_PLT)						      \
   | (((type) == R_TILE(COPY)) * ELF_RTYPE_CLASS_COPY))

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT R_TILE(JMP_SLOT)

/* TILE never uses Elf32_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1
#define ELF_MACHINE_NO_RELA 0

/* We define an initialization functions.  This is called very early in
   _dl_sysdep_start.  */
#define DL_PLATFORM_INIT dl_platform_init ()

static inline void __attribute__ ((unused))
dl_platform_init (void)
{
  if (GLRO(dl_platform) != NULL && *GLRO(dl_platform) == '\0')
    /* Avoid an empty string which would disturb us.  */
    GLRO(dl_platform) = NULL;
}

static inline ElfW(Addr)
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const ElfW(Rela) *reloc,
		       ElfW(Addr) *reloc_addr, ElfW(Addr) value)
{
  return *reloc_addr = value;
}

/* Return the final value of a plt relocation.  */
static inline ElfW(Addr)
elf_machine_plt_value (struct link_map *map, const ElfW(Rela) *reloc,
		       ElfW(Addr) value)
{
  return value;
}

/* Support notifying the simulator about new objects. */
void internal_function _dl_after_load (struct link_map *l);
#define DL_AFTER_LOAD _dl_after_load

/* Names of the architecture-specific auditing callback functions.  */
#define ARCH_LA_PLTENTER tile_gnu_pltenter
#define ARCH_LA_PLTEXIT tile_gnu_pltexit

#endif /* !dl_machine_h */


#ifdef RESOLVE_MAP

struct reloc_howto
{
  /* Right shift operand by this number of bits. */
  unsigned char right_shift;

#ifdef __tilegx__
  /* If nonzero, this is updating a code bundle. */
  unsigned char is_bundle_update;
#else
  /* If nonzero, add 0x8000 to the value. */
  unsigned char add_0x8000;
#endif

  /* If nonzero, subtract the containing address from the address. */
  unsigned char is_pcrel;

  /* Size in bytes, or 0 if this table entry should be ignored. */
  unsigned char byte_size;
};

/* Relocation information. Cannot contain create_* function pointers
   because then the table would not be position-independent. */
static const struct reloc_howto howto[] =
{
#ifdef __tilegx__

# if __WORDSIZE == 32
  /* The GX -m32 loader only handles 32-bit types, so it will be confused
     by shifts larger than that.  We convert them to just sign-extend;
     they usually indicate a program bug or missed optimization, but we
     have to handle them correctly anyway.  */
#  define S32 31
#  define S48 31
# else
#  define S32 32
#  define S48 48
# endif

  /* R_TILEGX_NONE */                    {   0, 0, 0, 0 },
  /* R_TILEGX_64 */                      {   0, 0, 0, 8 },
  /* R_TILEGX_32 */                      {   0, 0, 0, 4 },
  /* R_TILEGX_16 */                      {   0, 0, 0, 2 },
  /* R_TILEGX_8 */                       {   0, 0, 0, 1 },
  /* R_TILEGX_64_PCREL */                {   0, 0, 1, 8 },
  /* R_TILEGX_32_PCREL */                {   0, 0, 1, 4 },
  /* R_TILEGX_16_PCREL */                {   0, 0, 1, 2 },
  /* R_TILEGX_8_PCREL */                 {   0, 0, 1, 1 },
  /* R_TILEGX_HW0 */                     {   0, 0, 0, 0 },
  /* R_TILEGX_HW1 */                     {  16, 0, 0, 0 },
  /* R_TILEGX_HW2 */                     { S32, 0, 0, 0 },
  /* R_TILEGX_HW3 */                     { S48, 0, 0, 0 },
  /* R_TILEGX_HW0_LAST */                {   0, 0, 0, 0 },
  /* R_TILEGX_HW1_LAST */                {  16, 0, 0, 0 },
  /* R_TILEGX_HW2_LAST */                { S32, 0, 0, 0 },
  /* R_TILEGX_COPY */                    {   0, 0, 0, 0 },
  /* R_TILEGX_GLOB_DAT */                {   0, 0, 0, 8 },
  /* R_TILEGX_JMP_SLOT */                {   0, 0, 0, 0 },
  /* R_TILEGX_RELATIVE */                {   0, 0, 0, 0 },
  /* R_TILEGX_BROFF_X1 */                {   3, 1, 1, 8 },
  /* R_TILEGX_JUMPOFF_X1 */              {   3, 1, 1, 8 },
  /* R_TILEGX_JUMPOFF_X1_PLT */          {   3, 1, 1, 8 },
  /* R_TILEGX_IMM8_X0 */                 {   0, 1, 0, 8 },
  /* R_TILEGX_IMM8_Y0 */                 {   0, 1, 0, 8 },
  /* R_TILEGX_IMM8_X1 */                 {   0, 1, 0, 8 },
  /* R_TILEGX_IMM8_Y1 */                 {   0, 1, 0, 8 },
  /* R_TILEGX_DEST_IMM8_X1 */            {   0, 1, 0, 8 },
  /* R_TILEGX_MT_IMM14_X1 */             {   0, 1, 0, 8 },
  /* R_TILEGX_MF_IMM14_X1 */             {   0, 1, 0, 8 },
  /* R_TILEGX_MMSTART_X0 */              {   0, 1, 0, 8 },
  /* R_TILEGX_MMEND_X0 */                {   0, 1, 0, 8 },
  /* R_TILEGX_SHAMT_X0 */                {   0, 1, 0, 8 },
  /* R_TILEGX_SHAMT_X1 */                {   0, 1, 0, 8 },
  /* R_TILEGX_SHAMT_Y0 */                {   0, 1, 0, 8 },
  /* R_TILEGX_SHAMT_Y1 */                {   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW0 */            {   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW0 */            {   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW1 */            {  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW1 */            {  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW2 */            { S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW2 */            { S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW3 */            { S48, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW3 */            { S48, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW0_LAST */       {   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW0_LAST */       {   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW1_LAST */       {  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW1_LAST */       {  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW2_LAST */       { S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW2_LAST */       { S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW0_PCREL */      {   0, 1, 1, 8 },
  /* R_TILEGX_IMM16_X1_HW0_PCREL */      {   0, 1, 1, 8 },
  /* R_TILEGX_IMM16_X0_HW1_PCREL */      {  16, 1, 1, 8 },
  /* R_TILEGX_IMM16_X1_HW1_PCREL */      {  16, 1, 1, 8 },
  /* R_TILEGX_IMM16_X0_HW2_PCREL */      { S32, 1, 1, 8 },
  /* R_TILEGX_IMM16_X1_HW2_PCREL */      { S32, 1, 1, 8 },
  /* R_TILEGX_IMM16_X0_HW3_PCREL */      { S48, 1, 1, 8 },
  /* R_TILEGX_IMM16_X1_HW3_PCREL */      { S48, 1, 1, 8 },
  /* R_TILEGX_IMM16_X0_HW0_LAST_PCREL */ {   0, 1, 1, 8 },
  /* R_TILEGX_IMM16_X1_HW0_LAST_PCREL */ {   0, 1, 1, 8 },
  /* R_TILEGX_IMM16_X0_HW1_LAST_PCREL */ {  16, 1, 1, 8 },
  /* R_TILEGX_IMM16_X1_HW1_LAST_PCREL */ {  16, 1, 1, 8 },
  /* R_TILEGX_IMM16_X0_HW2_LAST_PCREL */ { S32, 1, 1, 8 },
  /* R_TILEGX_IMM16_X1_HW2_LAST_PCREL */ { S32, 1, 1, 8 },
  /* R_TILEGX_IMM16_X0_HW0_GOT */        {   0, 1, 0, 0 },
  /* R_TILEGX_IMM16_X1_HW0_GOT */        {   0, 1, 0, 0 },
  /* R_TILEGX_IMM16_X0_HW1_GOT */        {  16, 1, 0, 0 },
  /* R_TILEGX_IMM16_X1_HW1_GOT */        {  16, 1, 0, 0 },
  /* R_TILEGX_IMM16_X0_HW2_GOT */        { S32, 1, 0, 0 },
  /* R_TILEGX_IMM16_X1_HW2_GOT */        { S32, 1, 0, 0 },
  /* R_TILEGX_IMM16_X0_HW3_GOT */        { S48, 1, 0, 0 },
  /* R_TILEGX_IMM16_X1_HW3_GOT */        { S48, 1, 0, 0 },
  /* R_TILEGX_IMM16_X0_HW0_LAST_GOT */   {   0, 1, 0, 0 },
  /* R_TILEGX_IMM16_X1_HW0_LAST_GOT */   {   0, 1, 0, 0 },
  /* R_TILEGX_IMM16_X0_HW1_LAST_GOT */   {  16, 1, 0, 0 },
  /* R_TILEGX_IMM16_X1_HW1_LAST_GOT */   {  16, 1, 0, 0 },
  /* R_TILEGX_IMM16_X0_HW2_LAST_GOT */   { S32, 1, 0, 0 },
  /* R_TILEGX_IMM16_X1_HW2_LAST_GOT */   { S32, 1, 0, 0 },
  /* R_TILEGX_IMM16_X0_HW0_TLS_GD */     {   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW0_TLS_GD */     {   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW1_TLS_GD */     {  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW1_TLS_GD */     {  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW2_TLS_GD */     { S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW2_TLS_GD */     { S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW3_TLS_GD */     { S48, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW3_TLS_GD */     { S48, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD */{   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD */{   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD */{  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD */{  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW2_LAST_TLS_GD */{ S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW2_LAST_TLS_GD */{ S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW0_TLS_IE */     {   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW0_TLS_IE */     {   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW1_TLS_IE */     {  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW1_TLS_IE */     {  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW2_TLS_IE */     { S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW2_TLS_IE */     { S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW3_TLS_IE */     { S48, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW3_TLS_IE */     { S48, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE */{   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE */{   0, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE */{  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE */{  16, 1, 0, 8 },
  /* R_TILEGX_IMM16_X0_HW2_LAST_TLS_IE */{ S32, 1, 0, 8 },
  /* R_TILEGX_IMM16_X1_HW2_LAST_TLS_IE */{ S32, 1, 0, 8 },
  /* R_TILEGX_TLS_DTPMOD64 */            {   0, 0, 0, 0 },
  /* R_TILEGX_TLS_DTPOFF64 */            {   0, 0, 0, 0 },
  /* R_TILEGX_TLS_TPOFF64 */             {   0, 0, 0, 0 },
  /* R_TILEGX_TLS_DTPMOD32 */            {   0, 0, 0, 0 },
  /* R_TILEGX_TLS_DTPOFF32 */            {   0, 0, 0, 0 },
  /* R_TILEGX_TLS_TPOFF32 */             {   0, 0, 0, 0 }
#else
  /* R_TILEPRO_NONE */                   { 0,  0, 0, 0 },
  /* R_TILEPRO_32 */                     { 0,  0, 0, 4 },
  /* R_TILEPRO_16 */                     { 0,  0, 0, 2 },
  /* R_TILEPRO_8 */                      { 0,  0, 0, 1 },
  /* R_TILEPRO_32_PCREL */               { 0,  0, 1, 4 },
  /* R_TILEPRO_16_PCREL */               { 0,  0, 1, 2 },
  /* R_TILEPRO_8_PCREL */                { 0,  0, 1, 1 },
  /* R_TILEPRO_LO16 */                   { 0,  0, 0, 2 },
  /* R_TILEPRO_HI16 */                   { 16, 0, 0, 2 },
  /* R_TILEPRO_HA16 */                   { 16, 1, 0, 2 },
  /* R_TILEPRO_COPY */                   { 0,  0, 0, 0 },
  /* R_TILEPRO_GLOB_DAT */               { 0,  0, 0, 4 },
  /* R_TILEPRO_JMP_SLOT */               { 0,  0, 0, 0 },
  /* R_TILEPRO_RELATIVE */               { 0,  0, 0, 0 },
  /* R_TILEPRO_BROFF_X1 */               { 3,  0, 1, 8 },
  /* R_TILEPRO_JOFFLONG_X1 */            { 3,  0, 1, 8 },
  /* R_TILEPRO_JOFFLONG_X1_PLT */        { 3,  0, 1, 8 },
  /* R_TILEPRO_IMM8_X0 */                { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM8_Y0 */                { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM8_X1 */                { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM8_Y1 */                { 0,  0, 0, 8 },
  /* R_TILEPRO_MT_IMM15_X1 */            { 0,  0, 0, 8 },
  /* R_TILEPRO_MF_IMM15_X1 */            { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X0 */               { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X1 */               { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X0_LO */            { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X1_LO */            { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X0_HI */            { 16, 0, 0, 8 },
  /* R_TILEPRO_IMM16_X1_HI */            { 16, 0, 0, 8 },
  /* R_TILEPRO_IMM16_X0_HA */            { 16, 1, 0, 8 },
  /* R_TILEPRO_IMM16_X1_HA */            { 16, 1, 0, 8 },
  /* R_TILEPRO_IMM16_X0_PCREL */         { 0,  0, 1, 8 },
  /* R_TILEPRO_IMM16_X1_PCREL */         { 0,  0, 1, 8 },
  /* R_TILEPRO_IMM16_X0_LO_PCREL */      { 0,  0, 1, 8 },
  /* R_TILEPRO_IMM16_X1_LO_PCREL */      { 0,  0, 1, 8 },
  /* R_TILEPRO_IMM16_X0_HI_PCREL */      { 16, 0, 1, 8 },
  /* R_TILEPRO_IMM16_X1_HI_PCREL */      { 16, 0, 1, 8 },
  /* R_TILEPRO_IMM16_X0_HA_PCREL */      { 16, 1, 1, 8 },
  /* R_TILEPRO_IMM16_X1_HA_PCREL */      { 16, 1, 1, 8 },
  /* R_TILEPRO_IMM16_X0_GOT */           { 0,  0, 0, 0 },
  /* R_TILEPRO_IMM16_X1_GOT */           { 0,  0, 0, 0 },
  /* R_TILEPRO_IMM16_X0_GOT_LO */        { 0,  0, 0, 0 },
  /* R_TILEPRO_IMM16_X1_GOT_LO */        { 0,  0, 0, 0 },
  /* R_TILEPRO_IMM16_X0_GOT_HI */        { 0,  0, 0, 0 },
  /* R_TILEPRO_IMM16_X1_GOT_HI */        { 0,  0, 0, 0 },
  /* R_TILEPRO_IMM16_X0_GOT_HA */        { 0,  0, 0, 0 },
  /* R_TILEPRO_IMM16_X1_GOT_HA */        { 0,  0, 0, 0 },
  /* R_TILEPRO_MMSTART_X0 */             { 0,  0, 0, 8 },
  /* R_TILEPRO_MMEND_X0 */               { 0,  0, 0, 8 },
  /* R_TILEPRO_MMSTART_X1 */             { 0,  0, 0, 8 },
  /* R_TILEPRO_MMEND_X1 */               { 0,  0, 0, 8 },
  /* R_TILEPRO_SHAMT_X0 */               { 0,  0, 0, 8 },
  /* R_TILEPRO_SHAMT_X1 */               { 0,  0, 0, 8 },
  /* R_TILEPRO_SHAMT_Y0 */               { 0,  0, 0, 8 },
  /* R_TILEPRO_SHAMT_Y1 */               { 0,  0, 0, 8 },
  /* R_TILEPRO_SN_BROFF */               { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_IMM8 */                { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_UIMM8 */               { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_BYTE0 */               { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_BYTE1 */               { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_BYTE2 */               { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_BYTE3 */               { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_SPCREL0 */             { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_SPCREL1 */             { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_SPCREL2 */             { 0,  0, 0, 0 },
  /* R_TILEPRO_SN_SPCREL3 */             { 0,  0, 0, 0 },
  /* R_TILEPRO_IMM16_X0_TLS_GD */        { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X1_TLS_GD */        { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X0_TLS_GD_LO */     { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X1_TLS_GD_LO */     { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X0_TLS_GD_HI */     { 16, 0, 0, 8 },
  /* R_TILEPRO_IMM16_X1_TLS_GD_HI */     { 16, 0, 0, 8 },
  /* R_TILEPRO_IMM16_X0_TLS_GD_HA */     { 16, 1, 0, 8 },
  /* R_TILEPRO_IMM16_X1_TLS_GD_HA */     { 16, 1, 0, 8 },
  /* R_TILEPRO_IMM16_X0_TLS_IE */        { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X1_TLS_IE */        { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X0_TLS_IE_LO */     { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X1_TLS_IE_LO */     { 0,  0, 0, 8 },
  /* R_TILEPRO_IMM16_X0_TLS_IE_HI */     { 16, 0, 0, 8 },
  /* R_TILEPRO_IMM16_X1_TLS_IE_HI */     { 16, 0, 0, 8 },
  /* R_TILEPRO_IMM16_X0_TLS_IE_HA */     { 16, 1, 0, 8 },
  /* R_TILEPRO_IMM16_X1_TLS_IE_HA */     { 16, 1, 0, 8 },
  /* R_TILEPRO_TLS_DTPMOD32 */           { 0,  0, 0, 0 },
  /* R_TILEPRO_TLS_DTPOFF32 */           { 0,  0, 0, 0 },
  /* R_TILEPRO_TLS_TPOFF32 */            { 0,  0, 0, 0 },
#endif
};

#if __ELF_NATIVE_CLASS == 32
#define ELFW_R_TYPE	ELF32_R_TYPE
#define ELFW_ST_TYPE	ELF32_ST_TYPE
#else
#define ELFW_R_TYPE	ELF64_R_TYPE
#define ELFW_ST_TYPE	ELF64_ST_TYPE
#endif

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

auto inline void __attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, const ElfW(Rela) *reloc,
		  const ElfW(Sym) *sym, const struct r_found_version *version,
		  void *const reloc_addr_arg, int skip_ifunc)
{
  ElfW(Addr) *const reloc_addr = reloc_addr_arg;
  const unsigned int r_type = ELFW_R_TYPE (reloc->r_info);

#if !defined RTLD_BOOTSTRAP || !defined HAVE_Z_COMBRELOC
  if (__builtin_expect (r_type == R_TILE(RELATIVE), 0))
    {
# if !defined RTLD_BOOTSTRAP && !defined HAVE_Z_COMBRELOC
      /* This is defined in rtld.c, but nowhere in the static libc.a;
         make the reference weak so static programs can still link.
         This declaration cannot be done when compiling rtld.c
         (i.e. #ifdef RTLD_BOOTSTRAP) because rtld.c contains the
         common defn for _dl_rtld_map, which is incompatible with a
         weak decl in the same file.  */
#  ifndef SHARED
      weak_extern (GL(dl_rtld_map));
#  endif
      if (map != &GL(dl_rtld_map)) /* Already done in rtld itself.  */
# endif
        *reloc_addr = map->l_addr + reloc->r_addend;
      return;
    }
#endif

  if (__builtin_expect (r_type == R_TILE(NONE), 0))
    return;

#if !defined RTLD_BOOTSTRAP && !defined RESOLVE_CONFLICT_FIND_MAP
  const ElfW(Sym) *const refsym = sym;
#endif
  struct link_map *sym_map = RESOLVE_MAP (&sym, version, r_type);
  ElfW(Addr) value;

  if (sym == NULL)
    value = 0;
  else if (ELFW_ST_TYPE (sym->st_info) == STT_SECTION)
    value = map->l_addr;  /* like a RELATIVE reloc */
  else
    value = sym_map->l_addr + sym->st_value;

  if (sym != NULL
      && __builtin_expect (ELFW(ST_TYPE) (sym->st_info) == STT_GNU_IFUNC, 0)
      && __builtin_expect (sym->st_shndx != SHN_UNDEF, 1)
      && __builtin_expect (!skip_ifunc, 1))
    value = ((Elf64_Addr (*) (void)) value) ();

  switch (r_type)
    {
    case R_TILE(JMP_SLOT):
      elf_machine_fixup_plt (map, 0, reloc, reloc_addr,
                             value + reloc->r_addend);
      return;

#ifndef RESOLVE_CONFLICT_FIND_MAP
    case R_TILE_TLS(DTPMOD):
# ifdef RTLD_BOOTSTRAP
      /* During startup the dynamic linker is always the module
         with index 1.
         XXX If this relocation is necessary move before RESOLVE
         call.  */
      *reloc_addr = 1;
# else
      /* Get the information from the link map returned by the
         resolv function.  */
      if (sym_map != NULL)
        *reloc_addr = sym_map->l_tls_modid;
# endif
      return;
    case R_TILE_TLS(DTPOFF):
# ifndef RTLD_BOOTSTRAP
      /* During relocation all TLS symbols are defined and used.
         Therefore the offset is already correct.  */
      if (sym != NULL)
        *reloc_addr = sym->st_value + reloc->r_addend;
# endif
      return;
    case R_TILE_TLS(TPOFF):
# ifdef RTLD_BOOTSTRAP
      *reloc_addr = sym->st_value + reloc->r_addend + map->l_tls_offset;
# else
      if (sym != NULL)
        {
          CHECK_STATIC_TLS (map, sym_map);
          *reloc_addr = (sym->st_value + reloc->r_addend
                         + sym_map->l_tls_offset);
        }
#endif
      return;
#endif /* use TLS */

#if !defined RTLD_BOOTSTRAP && !defined RESOLVE_CONFLICT_FIND_MAP
      /* Not needed in dl-conflict.c.  */
    case R_TILE(COPY):
      if (sym == NULL)
        /* This can happen in trace mode if an object could not be found.  */
        return;
      if (__builtin_expect (sym->st_size > refsym->st_size, 0)
          || (__builtin_expect (sym->st_size < refsym->st_size, 0)
              && __builtin_expect (GLRO(dl_verbose), 0)))
        {
          const char *strtab;

          strtab = (const char *) D_PTR (map,l_info[DT_STRTAB]);
          _dl_error_printf ("%s: Symbol `%s' has different size in shared"
                            " object, consider re-linking\n",
                            RTLD_PROGNAME, strtab + refsym->st_name);
        }
      memcpy (reloc_addr_arg, (void *) value,
              MIN (sym->st_size, refsym->st_size));
      return;
#endif
    }

  /* All remaining relocations must be in the lookup table. */
  const struct reloc_howto *h = &howto[r_type];
  if ((unsigned int) r_type >= sizeof howto / sizeof howto[0] ||
      h->byte_size == 0)
    {
#if !defined RTLD_BOOTSTRAP || defined _NDEBUG
      /* We add these checks in the version to relocate ld.so only
         if we are still debugging.  */
      _dl_reloc_bad_type (map, r_type, 0);
#endif
      return;
    }

  value += reloc->r_addend;

  /* The lookup table entry knows how to perform this reloc. */
  if (h->is_pcrel)
    value -= (ElfW(Addr)) reloc_addr;

#ifndef __tilegx__
  if (h->add_0x8000)
    value += 0x8000;
#endif

  value = ((long) value) >> h->right_shift;

  switch (h->byte_size)
    {
    case 1:
      *(char *) reloc_addr = value;
      return;
    case 2:
      *(short *) reloc_addr = value;
      return;
    case 4:
      *(int *) reloc_addr = value;
      return;
#ifdef __tilegx__
    case 8:
      if (!h->is_bundle_update)
        {
          *(ElfW(Addr) *) reloc_addr = value;
          return;
        }
#endif
    }

  /* We are updating a bundle, so use the function pointer that
     swizzles the operand bits into the right location. */

  tile_bundle_bits *p = (tile_bundle_bits *) reloc_addr;
  tile_bundle_bits bits = *p;

#define MUNGE_SIGNED(func, length) do {                             \
    bits = ((bits & ~create_##func (-1)) | create_##func (value));  \
    ElfW(Addr) result = get_##func (bits);                          \
    int signbits = __WORDSIZE - length;                             \
    result = (long) (result << signbits) >> signbits;               \
    if (result != value)                                            \
      _dl_signal_error (0, map->l_name, NULL,                       \
                        "relocation value too large for " #func);   \
  } while (0)

#define MUNGE(func) MUNGE_SIGNED(func, __WORDSIZE)

#define MUNGE_NOCHECK(func)                                     \
  bits = ((bits & ~create_##func (-1)) | create_##func (value))

  switch (r_type)
    {
#ifdef __tilegx__
    case R_TILEGX_BROFF_X1:
      MUNGE_SIGNED (BrOff_X1, 17);
      break;
    case R_TILEGX_JUMPOFF_X1:
    case R_TILEGX_JUMPOFF_X1_PLT:
      MUNGE_SIGNED (JumpOff_X1, 27);
      break;
    case R_TILEGX_IMM8_X0:
      MUNGE_SIGNED (Imm8_X0, 8);
      break;
    case R_TILEGX_IMM8_Y0:
      MUNGE_SIGNED (Imm8_Y0, 8);
      break;
    case R_TILEGX_IMM8_X1:
      MUNGE_SIGNED (Imm8_X1, 8);
      break;
    case R_TILEGX_IMM8_Y1:
      MUNGE_SIGNED (Imm8_Y1, 8);
      break;
    case R_TILEGX_MT_IMM14_X1:
      MUNGE (MT_Imm14_X1);
      break;
    case R_TILEGX_MF_IMM14_X1:
      MUNGE (MF_Imm14_X1);
      break;
    case R_TILEGX_IMM16_X0_HW0:
    case R_TILEGX_IMM16_X0_HW1:
    case R_TILEGX_IMM16_X0_HW2:
    case R_TILEGX_IMM16_X0_HW3:
    case R_TILEGX_IMM16_X0_HW0_PCREL:
    case R_TILEGX_IMM16_X0_HW1_PCREL:
    case R_TILEGX_IMM16_X0_HW2_PCREL:
    case R_TILEGX_IMM16_X0_HW3_PCREL:
    case R_TILEGX_IMM16_X0_HW0_TLS_GD:
    case R_TILEGX_IMM16_X0_HW0_TLS_IE:
      MUNGE_NOCHECK (Imm16_X0);
      break;
    case R_TILEGX_IMM16_X0_HW0_LAST:
    case R_TILEGX_IMM16_X0_HW1_LAST:
    case R_TILEGX_IMM16_X0_HW2_LAST:
    case R_TILEGX_IMM16_X0_HW0_LAST_PCREL:
    case R_TILEGX_IMM16_X0_HW1_LAST_PCREL:
    case R_TILEGX_IMM16_X0_HW2_LAST_PCREL:
    case R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD:
    case R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD:
    case R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE:
    case R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE:
      MUNGE_SIGNED (Imm16_X0, 16);
      break;
    case R_TILEGX_IMM16_X1_HW0:
    case R_TILEGX_IMM16_X1_HW1:
    case R_TILEGX_IMM16_X1_HW2:
    case R_TILEGX_IMM16_X1_HW3:
    case R_TILEGX_IMM16_X1_HW0_PCREL:
    case R_TILEGX_IMM16_X1_HW1_PCREL:
    case R_TILEGX_IMM16_X1_HW2_PCREL:
    case R_TILEGX_IMM16_X1_HW3_PCREL:
    case R_TILEGX_IMM16_X1_HW0_TLS_GD:
    case R_TILEGX_IMM16_X1_HW0_TLS_IE:
      MUNGE_NOCHECK (Imm16_X1);
      break;
    case R_TILEGX_IMM16_X1_HW0_LAST:
    case R_TILEGX_IMM16_X1_HW1_LAST:
    case R_TILEGX_IMM16_X1_HW2_LAST:
    case R_TILEGX_IMM16_X1_HW0_LAST_PCREL:
    case R_TILEGX_IMM16_X1_HW1_LAST_PCREL:
    case R_TILEGX_IMM16_X1_HW2_LAST_PCREL:
    case R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD:
    case R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD:
    case R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE:
    case R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE:
      MUNGE_SIGNED (Imm16_X1, 16);
      break;
    case R_TILEGX_MMSTART_X0:
      MUNGE (BFStart_X0);
      break;
    case R_TILEGX_MMEND_X0:
      MUNGE (BFEnd_X0);
      break;
    case R_TILEGX_SHAMT_X0:
      MUNGE (ShAmt_X0);
      break;
    case R_TILEGX_SHAMT_X1:
      MUNGE (ShAmt_X1);
      break;
    case R_TILEGX_SHAMT_Y0:
      MUNGE (ShAmt_Y0);
      break;
    case R_TILEGX_SHAMT_Y1:
      MUNGE (ShAmt_Y1);
      break;
#else
    case R_TILEPRO_BROFF_X1:
      MUNGE_SIGNED (BrOff_X1, 17);
      break;
    case R_TILEPRO_JOFFLONG_X1:
    case R_TILEPRO_JOFFLONG_X1_PLT:
      MUNGE_NOCHECK (JOffLong_X1);   /* holds full 32-bit value */
      break;
    case R_TILEPRO_IMM8_X0:
      MUNGE_SIGNED (Imm8_X0, 8);
      break;
    case R_TILEPRO_IMM8_Y0:
      MUNGE_SIGNED (Imm8_Y0, 8);
      break;
    case R_TILEPRO_IMM8_X1:
      MUNGE_SIGNED (Imm8_X1, 8);
      break;
    case R_TILEPRO_IMM8_Y1:
      MUNGE_SIGNED (Imm8_Y1, 8);
      break;
    case R_TILEPRO_MT_IMM15_X1:
      MUNGE (MT_Imm15_X1);
      break;
    case R_TILEPRO_MF_IMM15_X1:
      MUNGE (MF_Imm15_X1);
      break;
    case R_TILEPRO_IMM16_X0_LO:
    case R_TILEPRO_IMM16_X0_HI:
    case R_TILEPRO_IMM16_X0_HA:
    case R_TILEPRO_IMM16_X0_LO_PCREL:
    case R_TILEPRO_IMM16_X0_HI_PCREL:
    case R_TILEPRO_IMM16_X0_HA_PCREL:
    case R_TILEPRO_IMM16_X0_TLS_GD_LO:
    case R_TILEPRO_IMM16_X0_TLS_GD_HI:
    case R_TILEPRO_IMM16_X0_TLS_GD_HA:
    case R_TILEPRO_IMM16_X0_TLS_IE_LO:
    case R_TILEPRO_IMM16_X0_TLS_IE_HI:
    case R_TILEPRO_IMM16_X0_TLS_IE_HA:
      MUNGE_NOCHECK (Imm16_X0);
      break;
    case R_TILEPRO_IMM16_X0:
    case R_TILEPRO_IMM16_X0_PCREL:
    case R_TILEPRO_IMM16_X0_TLS_GD:
    case R_TILEPRO_IMM16_X0_TLS_IE:
      MUNGE_SIGNED (Imm16_X0, 16);
      break;
    case R_TILEPRO_IMM16_X1_LO:
    case R_TILEPRO_IMM16_X1_HI:
    case R_TILEPRO_IMM16_X1_HA:
    case R_TILEPRO_IMM16_X1_LO_PCREL:
    case R_TILEPRO_IMM16_X1_HI_PCREL:
    case R_TILEPRO_IMM16_X1_HA_PCREL:
    case R_TILEPRO_IMM16_X1_TLS_GD_LO:
    case R_TILEPRO_IMM16_X1_TLS_GD_HI:
    case R_TILEPRO_IMM16_X1_TLS_GD_HA:
    case R_TILEPRO_IMM16_X1_TLS_IE_LO:
    case R_TILEPRO_IMM16_X1_TLS_IE_HI:
    case R_TILEPRO_IMM16_X1_TLS_IE_HA:
      MUNGE_NOCHECK (Imm16_X1);
      break;
    case R_TILEPRO_IMM16_X1:
    case R_TILEPRO_IMM16_X1_PCREL:
    case R_TILEPRO_IMM16_X1_TLS_GD:
    case R_TILEPRO_IMM16_X1_TLS_IE:
      MUNGE_SIGNED (Imm16_X1, 16);
      break;
    case R_TILEPRO_MMSTART_X0:
      MUNGE (MMStart_X0);
      break;
    case R_TILEPRO_MMEND_X0:
      MUNGE (MMEnd_X0);
      break;
    case R_TILEPRO_MMSTART_X1:
      MUNGE (MMStart_X1);
      break;
    case R_TILEPRO_MMEND_X1:
      MUNGE (MMEnd_X1);
      break;
    case R_TILEPRO_SHAMT_X0:
      MUNGE (ShAmt_X0);
      break;
    case R_TILEPRO_SHAMT_X1:
      MUNGE (ShAmt_X1);
      break;
    case R_TILEPRO_SHAMT_Y0:
      MUNGE (ShAmt_Y0);
      break;
    case R_TILEPRO_SHAMT_Y1:
      MUNGE (ShAmt_Y1);
      break;
#endif
    }
#undef MUNGE
  *p = bits;
  _dl_flush_icache (p, sizeof (*p));
}

auto inline void __attribute__ ((always_inline))
elf_machine_rela_relative (ElfW(Addr) l_addr, const ElfW(Rela) *reloc,
                           void *const reloc_addr_arg)
{
  ElfW(Addr) *const reloc_addr = reloc_addr_arg;
  *reloc_addr = l_addr + reloc->r_addend;
}

auto inline void __attribute__ ((always_inline))
elf_machine_lazy_rel (struct link_map *map,
                      ElfW(Addr) l_addr, const ElfW(Rela) *reloc,
                      int skip_ifunc)
{
  const unsigned int r_type = ELF32_R_TYPE (reloc->r_info);

  /* Check for unexpected PLT reloc type.  */
  if (__builtin_expect (r_type == R_TILE(JMP_SLOT), 1))
    {
      *(ElfW(Addr) *) (l_addr + reloc->r_offset) += l_addr;
    }
  else
    _dl_reloc_bad_type (map, r_type, 1);
}

#endif /* RESOLVE_MAP */
