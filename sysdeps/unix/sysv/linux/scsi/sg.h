/* Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
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

#define SG_MAX_SENSE	16

struct sg_header
 {
   /* Length of incoming packet (including header).  */
   int pack_len;
   /* Maximal length of expected reply.  */
   int reply_len;
   /* Id number of packet.  */
   int pack_id;
   /* 0==ok, otherwise error number.  */
   int result;
   /* Force 12 byte command length for group 6 & 7 commands.  */
   unsigned int twelve_byte:1;
   /* SCSI status from target.  */
   unsigned int target_status:5;
   /* Host status (see "DID" codes).  */
   unsigned int host_status:8;
   /* Driver status+suggestion.  */
   unsigned int driver_status:8;
   /* Unused.  */
   unsigned int other_flags:10;
   /* Output in 3 cases:
      when target_status is CHECK_CONDITION or
      when target_status is COMMAND_TERMINATED or
      when (driver_status & DRIVER_SENSE) is true.  */
   unsigned char sense_buffer[SG_MAX_SENSE];
 };

/* Request information about a specific SG device.  */
struct sg_scsi_id {
  /* Host number as in "scsi<n>" where 'n' is one of 0, 1, 2 etc.  */
  int host_no;
  int channel;
  /* SCSI id of target device.  */
  int scsi_id;
  int lun;
  /* TYPE_... defined in <scsi/scsi.h>.  */
  int scsi_type;
  /* Host (adapter) maximum commands per lun.  */
  short int h_cmd_per_lun;
  /* Device (or adapter) maximum queue length.  */
  short int d_queue_depth;
  /* Unused, set to 0 for now.  */
  int unused1;
  /* Unused, set to 0 for now.  */
  int unused2;
};

/* Ioctl's */
#define SG_SET_TIMEOUT		0x2201	/* Set timeout; *(int *)arg==timeout.  */
#define SG_GET_TIMEOUT		0x2202	/* Get timeout; return timeout.  */

#define SG_EMULATED_HOST	0x2203	/* True for emulated host adapter (ATAPI). */

/* Used to configure SCSI command transformation layer for ATAPI devices.  */
#define SG_SET_TRANSFORM	0x2204
#define SG_GET_TRANSFORM	0x2205

#define SG_SET_RESERVED_SIZE	0x2275  /* Request a new reserved buffer size.  */
#define SG_GET_RESERVED_SIZE	0x2272  /* Get actual size of reserved buffer */

/* The following ioctl takes a 'sg_scsi_id *' object as its 3rd argument. */
#define SG_GET_SCSI_ID		0x2276	/* Yields fd's bus, chan, dev, lun + type
					   SCSI id information can also be obtained
					   from SCSI_IOCTL_GET_IDLUN. */

/* Override host setting and always DMA using low memory ( <16MB on i386).  */
#define SG_SET_FORCE_LOW_DMA	0x2279	/* 0-> use adapter setting, 1-> force  */
#define SG_GET_LOW_DMA		0x227a	/* 0-> use all ram for dma; 1-> low dma ram */

/* If SG_SET_FORCE_PACK_ID is set to 1, pack_id is input to read() which
   will attempt to read that pack_id or block (or return EAGAIN).  If
   pack_id is -1 then read oldest waiting.  If SG_SET_FORCE_PACK_ID is
   set to 0 then pack_id gets ignored by read() and the oldest
   readable is fetched.  */
#define SG_SET_FORCE_PACK_ID	0x227b
#define SG_GET_PACK_ID		0x227c	/* Yields oldest readable pack_id (or -1).  */

#define SG_GET_NUM_WAITING	0x227d	/* Number of commands awaiting read().  */

/* Turn on error sense trace (1..8), dump this device to log/console (9)
   or dump all sg device states ( >9 ) to log/console.  */
#define SG_SET_DEBUG		0x227e	/* 0 -> turn off debug */

/* Yields max scatter gather tablesize allowed by current host adapter.  */
#define SG_GET_SG_TABLESIZE	0x227F	/* 0 implies can't do scatter gather */

/* Control whether sequencing per file descriptor or per device.  */
#define SG_GET_MERGE_FD		0x2274	/* 0-> per fd, 1-> per device */
#define SG_SET_MERGE_FD		0x2273	/* Attempt to change sequencing state,
					   if more than current fd open on device,
					   will fail with EBUSY.  */

/* Get/set command queuing state per fd (default is SG_DEF_COMMAND_Q). */
#define SG_GET_COMMAND_Q	0x2270	/* Yields 0 (queuing off) or 1 (on).  */
#define SG_SET_COMMAND_Q 	0x2271	/* Change queuing state with 0 or 1.  */

/* Get/set whether DMA underrun will cause an error (DID_ERROR).  Currently this
   only applies to the [much-used] aic7xxx driver.  */
#define SG_GET_UNDERRUN_FLAG	0x2280	/* Yields 0 (don't flag) or 1 (flag). */
#define SG_SET_UNDERRUN_FLAG	0x2281	/* Change flag underrun state.  */

#define SG_GET_VERSION_NUM	0x2282	/* Example: version 2.1.34 yields 20134 */
#define SG_NEXT_CMD_LEN		0x2283	/* Override SCSI command length with given
					   number on the next write() on this file
					   descriptor.  */

/* Returns -EBUSY if occupied else takes as input: 0 -> do nothing,
   1 -> device reset or  2 -> bus reset (may not be activated yet).  */
#define SG_SCSI_RESET		0x2284


/* Largest size (in bytes) a single scatter-gather list element can have.
   The value must be a power of 2 and <= (PAGE_SIZE * 32) [131072 bytes on 
   i386].  The minimum value is PAGE_SIZE.  If scatter-gather not supported
   by adapter then this value is the largest data block that can be
   read/written by a single scsi command.  The user can find the value of
   PAGE_SIZE by calling getpagesize() defined in <unistd.h>.  */
#define SG_SCATTER_SZ 		(8 * 4096) /* PAGE_SIZE is not available to user.  */

#define SG_DEFAULT_TIMEOUT	(60 * HZ) /* 1 minute timeout */
#define SG_DEFAULT_RETRIES	1

/* Defaults, commented if they differ from original sg driver */
#define SG_DEF_COMMAND_Q 	0
#define SG_DEF_MERGE_FD		0	/* was 1 -> per device sequencing */
#define SG_DEF_FORCE_LOW_DMA	0	/* was 1 -> memory below 16MB on i386 */
#define SG_DEF_FORCE_PACK_ID	0
#define SG_DEF_UNDERRUN_FLAG	0
#define SG_DEF_RESERVED_SIZE	SG_SCATTER_SZ

#define SG_MAX_QUEUE 		16	/* Maximum outstanding request, arbitrary,
					   may be changed if sufficient DMA buffer
					   room available.  */

#define SG_BIG_BUFF SG_DEF_RESERVED_SIZE  /* for backward compatibility */

#endif	/* scsi/sg.h */
