/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

/*
   History:
    Started: Aug 9 by Lawrence Foard (entropy@world.std.com), to allow user
     process control of SCSI devices.
    Development Sponsored by Killy Corp. NY NY
*/

#ifndef _SCSI_SG_H
#define _SCSI_SG_H	1

#include <features.h>

/* An SG device is accessed by writing "packets" to it, the replies
   are then read using the read call.  The same header is used for
   replies; ignore the reply_len field.  */

struct sg_header
 {
   int pack_len;			/* length of incoming packet
					   (including header).  */
   int reply_len;			/* max length of expected reply.  */
   int pack_id;				/* id number of packet.  */
   int result;				/* 0==ok, otherwise error number.  */
   unsigned int twelve_byte:1;		/* force 12 byte command length for
					   group 6 & 7 commands.  */
   unsigned int other_flags:31;		/* for future use.  */
   unsigned char sense_buffer[16];	/* used only by reads. */
   /* command follows then data for command.  */
 };

/* ioctl's */
#define SG_SET_TIMEOUT 0x2201  /* set timeout *(int *)arg==timeout */
#define SG_GET_TIMEOUT 0x2202  /* get timeout return timeout */

#define SG_DEFAULT_TIMEOUT (60*HZ) /* 1 minute timeout */
#define SG_DEFAULT_RETRIES 1

#define SG_MAX_QUEUE 4 /* maximum outstanding request, arbitrary, may be
			  changed if sufficient DMA buffer room available.  */

#define SG_BIG_BUFF 32768

#endif	/* scsi/sg.h */
