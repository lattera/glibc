/* Copyright (C) 1996, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef	_SYS_IO_H
#define	_SYS_IO_H	1

#include <features.h>

__BEGIN_DECLS

/* If TURN_ON is TRUE, request for permission to do direct i/o on the
   port numbers in the range [FROM,FROM+NUM-1].  Otherwise, turn I/O
   permission off for that range.  This call requires root privileges.

   Portability note: not all Linux platforms support this call.  Most
   platforms based on the PC I/O architecture probably will, however.
   E.g., Linux/Alpha for Alpha PCs supports this.  */
extern int ioperm (unsigned long int __from, unsigned long int __num,
                   int __turn_on) __THROW;

/* Set the I/O privilege level to LEVEL.  If LEVEL>3, permission to
   access any I/O port is granted.  This call requires root
   privileges. */
extern int iopl (int __level) __THROW;


extern inline unsigned char
inb (unsigned short port)
{
  unsigned char _v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

extern inline unsigned char
inb_p (unsigned short port)
{
  unsigned char _v;

  __asm__ __volatile__ ("inb %w1,%0\noutb %%al,$0x80":"=a" (_v):"Nd" (port));
  return _v;
}

extern inline unsigned short
inw (unsigned short port)
{
  unsigned short _v;

  __asm__ __volatile__ ("inw %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

extern inline unsigned short
inw_p (unsigned short port)
{
  unsigned short _v;

  __asm__ __volatile__ ("inw %w1,%0\noutb %%al,$0x80":"=a" (_v):"Nd" (port));
  return _v;
}

extern inline unsigned int
inl (unsigned short port)
{
  unsigned int _v;

  __asm__ __volatile__ ("inl %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

extern inline unsigned int
inl_p (unsigned short port)
{
  unsigned int _v;
  __asm__ __volatile__ ("inl %w1,%0\noutb %%al,$0x80":"=a" (_v):"Nd" (port));
  return _v;
}

extern inline void
outb (unsigned char value, unsigned short port)
{
  __asm__ __volatile__ ("outb %b0,%w1"::"a" (value), "Nd" (port));
}

extern inline void
outb_p (unsigned char value, unsigned short port)
{
  __asm__ __volatile__ ("outb %b0,%w1\noutb %%al,$0x80"::"a" (value),
			"Nd" (port));
}

extern inline void
outw (unsigned short value, unsigned short port)
{
  __asm__ __volatile__ ("outw %w0,%w1"::"a" (value), "Nd" (port));

}

extern inline void
outw_p (unsigned short value, unsigned short port)
{
  __asm__ __volatile__ ("outw %w0,%w1\noutb %%al,$0x80"::"a" (value),
			"Nd" (port));
}

extern inline void
outl (unsigned int value, unsigned short port)
{
  __asm__ __volatile__ ("outl %0,%w1"::"a" (value), "Nd" (port));
}

extern inline void
outl_p (unsigned int value, unsigned short port)
{
  __asm__ __volatile__ ("outl %0,%w1\noutb %%al,$0x80"::"a" (value),
			"Nd" (port));
}

extern inline void
insb (unsigned short port, void *addr, unsigned long count)
{
  __asm__ __volatile__ ("cld ; rep ; insb":"=D" (addr),
			"=c" (count):"d" (port), "0" (addr), "1" (count));
}

extern inline void
insw (unsigned short port, void *addr, unsigned long count)
{
  __asm__ __volatile__ ("cld ; rep ; insw":"=D" (addr),
			"=c" (count):"d" (port), "0" (addr), "1" (count));
}

extern inline void
insl (unsigned short port, void *addr, unsigned long count)
{
  __asm__ __volatile__ ("cld ; rep ; insl":"=D" (addr),
			"=c" (count):"d" (port), "0" (addr), "1" (count));
}

extern inline void
outsb (unsigned short port, const void *addr, unsigned long count)
{
  __asm__ __volatile__ ("cld ; rep ; outsb":"=S" (addr),
			"=c" (count):"d" (port), "0" (addr), "1" (count));
}

extern inline void
outsw (unsigned short port, const void *addr, unsigned long count)
{
  __asm__ __volatile__ ("cld ; rep ; outsw":"=S" (addr),
			"=c" (count):"d" (port), "0" (addr), "1" (count));
}

extern inline void
outsl (unsigned short port, const void *addr, unsigned long count)
{
  __asm__ __volatile__ ("cld ; rep ; outsl":"=S" (addr),
			"=c" (count):"d" (port), "0" (addr), "1" (count));
}

__END_DECLS
#endif /* _SYS_IO_H */
