/* Interface for manipulating ELF object files; functions found in -lelf.
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

#ifndef _LIBELF_H
#define	_LIBELF_H 1

#include <sys/cdefs.h>
#include <sys/types.h>
#include <elf.h>

__BEGIN_DECLS

/* Commands to operate on an Elf descriptor.
   The meanings are slightly different for the different functions.  */
typedef enum 
{
  ELF_C_NULL = 0,
  ELF_C_READ,			/* Read from the file.  */
  ELF_C_WRITE,			/* Write the file, ignoring old contents.  */
  ELF_C_CLR,			/* Clear specified flag bits.  */
  ELF_C_SET,			/* Set specified flag bits.  */
  ELF_C_FDDONE,			/* Close the fd; no further io will happen.  */
  ELF_C_FDREAD,			/* Read the whole file, then close it.  */
  ELF_C_RDWR,			/* Read from and modify the file.  */
  ELF_C_NUM			/* Number of valid command values.  */
} Elf_Cmd;


/* Flag bits for `elf_flag*'.  */
#define	ELF_F_DIRTY	0x1	/* Object is  */
#define	ELF_F_LAYOUT	0x4


/* File types.  */
typedef enum
{
  ELF_K_NONE = 0,
  ELF_K_AR,
  ELF_K_COFF,			/* The GNU -lelf does not support COFF.  */
  ELF_K_ELF,
  ELF_K_NUM			/* Number of valid file kinds.  */
} Elf_Kind;


/* Translation types.  */
typedef enum
{
  ELF_T_BYTE = 0,
  ELF_T_ADDR,
  ELF_T_DYN,
  ELF_T_EHDR,
  ELF_T_HALF,
  ELF_T_OFF,
  ELF_T_PHDR,
  ELF_T_RELA,
  ELF_T_REL,
  ELF_T_SHDR,
  ELF_T_SWORD,
  ELF_T_SYM,
  ELF_T_WORD,
  ELF_T_NUM
} Elf_Type;


/* These types are opaque to user code; only pointers to them are used.  */
typedef struct Elf Elf;
typedef struct Elf_Scn Elf_Scn;


/* Archive member header.  */
typedef struct
{
  char *ar_name;
  time_t ar_date;
  uid_t ar_uid;
  gid_t ar_gid;
  mode_t ar_mode;
  off_t ar_size;
  char *ar_rawname;
} Elf_Arhdr;

/* Archive symbol table.  */
typedef struct
{
  char *as_name;
  size_t as_off;
  unsigned long int as_hash;
} Elf_Arsym;


/* Data descriptor.  */
typedef struct
{
  Elf_Void *d_buf;
  Elf_Type d_type;
  size_t d_size;
  off_t d_off;			/* Offset into section.  */
  size_t d_align;		/* Alignment in section.  */
  unsigned int d_version;	/* ELF version.  */
} Elf_Data;



/* Open an Elf descriptor on file descriptor FD.
   REF is the Elf descriptor for the containing archive (to open a member);
   or the descriptor previously returned for FD (to add a user reference);
   or NULL.  */
extern Elf *elf_begin __P ((int __fd, Elf_Cmd __cmd, Elf *__ref));

/* Finish using ELF (remove a user reference); if this is the last user
   reference, its data will be freed.  */
extern int elf_end __P ((Elf *__elf));

/* Control the library's access to the file descriptor for ELF.
   CMD is either ELF_C_FDDONE or ELF_C_FDREAD.  */
extern int elf_cntl __P ((Elf *__elf, Elf_Cmd __cmd));


/* Return a string describing an ELF error number.  */
extern __const char *elf_errmsg	__P ((int __errno));

/* Return the ELF error number for the last failed operation.  */
extern int elf_errno __P ((void));

/* Set the byte value used to fill sections for alignment.  */
extern void elf_fill __P ((int __fillchar));

/* The following functions `elf_flag*' all operate the same way:
   CMD is either ELF_C_SET or ELF_C_CLR; FLAGS are `ELF_F_*' above,
   which are set or cleared for the object the call relates to.  */

/* Modify flags affecting the file as a whole (?).  */
extern unsigned	int elf_flagelf __P ((Elf *__elf, Elf_Cmd __cmd,
				      unsigned int __flags));
/* Modify flags affecting DATA.  */
extern unsigned int elf_flagdata __P ((Elf_Data *__data, Elf_Cmd __cmd,
				       unsigned int __flags));
/* Modify flags affecting the ELF header.  */
extern unsigned	int elf_flagehdr __P ((Elf *__elf, Elf_Cmd __cmd,
				       unsigned int __flags));
/* Modify flags affecting the ELF program header.  */
extern unsigned	int elf_flagphdr __P ((Elf *__elf, Elf_Cmd, __cmd
				       unsigned int __flags));
/* Modify flags affecting the given section's data.  */
extern unsigned	int elf_flagscn __P ((Elf_Scn *__scn, Elf_Cmd __cmd,
				      unsigned int __flags));
/* Modify flags affecting the given section's header.  */
extern unsigned	int elf_flagshdr __P ((Elf_Scn *__scn, Elf_Cmd __cmd,
				       unsigned int __flags));


extern size_t elf32_fsize __P ((Elf_Type __type, size_t __count,
				unsigned int __ver));

/* Return the archive header for ELF, which must describe an archive.  */
extern Elf_Arhdr *elf_getarhdr __P ((Elf *__elf));

/* Return the archive symbol table for ELF, and store
   in *NELTSP the number of elements in the table.  */
extern Elf_Arsym *elf_getarsym __P ((Elf *__elf, size_t *__neltsp));

/* Return the file offset for the beginning of ELF.
   If ELF describes an archive member, this points to the member header.  */
extern off_t elf_getbase __P ((Elf *__elf));

/* Extract the data from a section.  */
extern Elf_Data *elf_getdata __P ((Elf_Scn *__scn, Elf_Data *__data));

/* Extract the ELF header from the file.  */
extern Elf32_Ehdr *elf32_getehdr __P ((Elf *__elf));

/* Extract the initial ELF identification bytes from the file.
   If PTR is nonnull, the number of identification bytes is stored there.  */
extern char *elf_getident __P((Elf *__elf, size_t *__ptr));

/* Extract the ELF program header from the file.  */
extern Elf32_Phdr *elf32_getphdr __P ((Elf *__elf));

/* Extract the indicated section from the file.  */
extern Elf_Scn *elf_getscn __P ((Elf *__elf, size_t __index));

/* Extract the section header from the section.  */
extern Elf32_Shdr *elf32_getshdr __P ((Elf_Scn *__scn));

/* Return the index of the section following SCN.  */
extern size_t elf_ndxscn __P ((Elf_Scn *__scn));


/* Standard ELF symbol name hash function.  */
extern unsigned long int elf_hash __P ((__const char *__name));

#if defined (__OPTIMIZE__) || defined (_EXTERN_INLINE)
#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE extern __inline
#endif
_EXTERN_INLINE unsigned long int
elf_hash (__const char *__name)
{
  /* This is the hashing function specified by the ELF ABI.  */
  unsigned long int __hash = 0;
  while (*__name != '\0')
    {
      unsigned long int __hi;
      __hash = (__hash << 4) + *__name++;
      __hi = __hash & 0xf0000000;
      if (__hi != 0)
	__hash ^= __hi >> 24;
      hash &= ~__hi;
    }
}
#endif

/* Return the kind of file ELF describes.  */
extern Elf_Kind	elf_kind __P ((Elf *__elf));

extern Elf_Data	*elf_newdata __P ((Elf_Scn *__scn));

/* Create the ELF header for ELF.  */
extern Elf32_Ehdr *elf32_newehdr __P ((Elf *__elf));

/* Create the program header for ELF, with COUNT segments.  */
extern Elf32_Phdr *elf32_newphdr __P ((Elf *__elf, size_t __count));

/* Create a new section in ELF.  */
extern Elf_Scn *elf_newscn __P ((Elf *__elf));

/* Return the section following SCN.  */
extern Elf_Scn *elf_nextscn __P ((Elf *__elf, Elf_Scn *__scn));

/* Set up ELF to read the next archive member.  */
extern Elf_Cmd elf_next __P ((Elf *__elf));

/* Set up ELF (which must describe an archive) to read the
   archive member that starts at file position OFFSET.  */
extern size_t elf_rand __P ((Elf *__elf, size_t __offset));

extern Elf_Data *elf_rawdata __P ((Elf_Scn *__scn, Elf_Data *__data));

/* Read the entire file into memory; store its size in *PTR.  */
extern char *elf_rawfile __P ((Elf *__elf, size_t *__ptr));

/* Return a pointer to the string at OFFSET bytes into the string table.
   SECTION is the index of the SHT_STRTAB section in ELF.  */
extern char *elf_strptr __P ((Elf *__elf, size_t __section, size_t __offset));

/* If CMD is ELF_C_NULL, update ELF's data structures based on any
   user modifications, and set the ELF_F_DIRTY flag if anything changed.
   If CMD is ELF_C_WRITE, do that and then write the changes to the file.
extern off_t elf_update __P ((Elf *__elf, Elf_Cmd __cmd));

/* Handle ELF version VER.  Return the old version handled,
   or EV_NONE if VER is unrecognized.  */
extern unsigned	int elf_version __P ((unsigned int __ver));

extern Elf_Data *elf32_xlatetof __P ((Elf_Data *__dst, const Elf_Data *__src,
				      unsigned int __encode));
extern Elf_Data *elf32_xlatetom __P ((Elf_Data *__dst, const Elf_Data *__src,
				      unsigned int __encode));

__END_DECLS

#endif	/* _LIBELF_H */
