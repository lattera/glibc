/* Copyright (C) 1998 Free Software Foundation, Inc.
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

#ifndef _BITS_STROPTS_H
#define _BITS_STROPTS_H	1

#include <bits/types.h>

/* Macros used as `request' argument to `ioctl'.  */
#define I_PUSH		1	/* Push STREAMS module onto top of the current
				   STREAM, just below the STREAM head.  */
#define I_POP		2	/* Remove STREAMS module from just below the
				   STREAM head.  */
#define I_LOOK		3	/* Retrieve the name of the module just below
				   the STREAM head and place it in a character
				   string.  */
#define I_FLUSH		4	/* Flush all input and/or output.  */
#define I_FLUSHBAND	5	/* Flush only band specified.  */
#define I_SETSIG	6	/* Inform the STREAM head that the process
				   wants the SIGPOLL signal issued.  */
#define I_GETSIG	7	/* Return the events for which the calling
				   process is currently registered to be sent
				   a SIGPOLL signal.  */
#define I_FIND		8	/* Compares the names of all modules currently
				   present in the STREAM to the name pointed to
				   by `arg'.  */
#define I_PEEK		9	/* Allows a process to retrieve the information
				   in the first message on the STREAM head read
				   queue without taking the message off the
				   queue.  */
#define I_SRDOPT	10	/* Sets the read mode.  */
#define I_GRDOPT	11	/* Returns the current read mode setting.  */
#define I_NREAD		12	/* Counts the number of data bytes in the data
				   block in the first message.  */
#define I_FDINSERT	13	/* Create a message from the specified
				   buffer(s), adds information about another
				   STREAM, and send the message downstream.  */
#define I_STR		14	/* Construct an internal STREAMS `ioctl'
				   message and send that message downstream. */
#define I_SWROPT	15	/* Set the write mode.  */
#define I_GWRSET	16	/* Return the current write mode setting.  */
#define I_SENDFD	17	/* Requests the STREAM associated with `fildes'
				   to send a message, containing a file
				   pointer, to the STREAM head at the other end
				   of a STREAMS pipe.  */
#define I_RECVFD	18	/* Retrieve the file descriptor associated with
				   the message sent by an I_SENDFD `ioctl'
				   over a STREAMS pipe.  */
#define I_LIST		19	/* List all the module names on the STREAM, up
				   to and including the topmost driver name. */
#define I_ATMARK	20	/* See if the current message on the STREAM
				   head read queue is "marked" by some module
				   downstream.  */
#define I_CKBAND	21	/* Check if the message of a given priority
				   band exists on the STREAM head read
				   queue.  */
#define I_GETBAND	22	/* Return the priority band of the first
				   message on the STREAM head read queue.  */
#define I_CANPUT	23	/* Check if a certain band is writable.  */
#define I_SETCLTIME	24	/* Set the time the STREAM head will delay when
				   a STREAM is closing and there is data on
				   the write queues.  */
#define I_LINK		25	/* Connect two STREAMs.  */
#define I_UNLINK	26	/* Disconnects the two STREAMs.  */
#define I_PLINK		27	/* Connect two STREAMs with a persistent
				   link.  */
#define I_PUNLINK	28	/* Disconnect the two STREAMs that were
				   connected with a persistent link.  */


/* Used in `I_LOOK' request.  */
#define FMNAMESZ	255

/* Flush options.  */
#define FLUSHR	1		/* Flush read queues.  */
#define FLUSHW	2		/* Flush write queues.  */
#define FLUSHRW	3		/* Flush read and write queues.  */

/* Possible arguments for `I_SETSIG'.  */
#define S_RDNORM	0x0001	/* A normal message has arrived.  */
#define S_RDBAND	0x0002	/* A message with a non-zero priority has
				   arrived.  */
#define S_INPUT		0x0004	/* A message, other than a high-priority
				   message, has arrived.  */
#define S_HIPRI		0x0008	/* A high-priority message is present.  */
#define S_OUTPUT	0x0010	/* The write queue for normal data is no longer
				   full.  */
#define S_WRNORM	S_OUTPUT
#define S_WRBAND	0x0020	/* The write queue for a non-zero priority
				   band is no longer full.  */
#define S_MSG		0x0040	/* A STREAMS signal message that contains the
				   SIGPOLL signal reaches the front of the
				   STREAM head read queue.  */
#define S_ERROR		0x0080	/* Notification of an error condition.  */
#define S_HANGUP	0x0100	/* Notification of a hangup.  */
#define S_BANDURG	0x0200	/* When used in conjunction with S_RDBAND,
				   SIGURG is generated instead of SIGPOLL when
				   a priority message reaches the front of the
				   STREAM head read queue.  */

/* Option for `I_PEEK'.  */
#define RS_HIPRI	1	/* Only look for high-priority messages.  */

/* Options for `I_SRDOPT'.  */
#define RDNORM		1	/* Byte-STREAM mode, the default.  */
#define RMSGD		2	/* Message-discard mode.   */
#define RMSGN		3	/* Message-nondiscard mode.   */
#define RPROTNORM	4	/* Fail `read' with EBADMSG if a message
				   containing a control part is at the front
				   of the STREAM head read queue.  */
#define RPROTDAT	5	/* Deliver the control part of a message as
				   data.  */
#define RPROTDIS	6	/* Discard the control part of a message,
				   delivering any data part.  */

/* Possible mode for `I_SWROPT'.  */
#define SNDZERO		1	/* Send a zero-length message downstream when a
				   `write' of 0 bytes occurs.  */

/* Arguments for `I_ATMARK'.  */
#define ANYMARK		1	/* Check if the message is marked.  */
#define LASTMARK	2	/* Check if the message is the last one marked
				   on the queue.  */

/* Argument for `I_UNLINK'.  */
#define MUXID_ALL	1	/* Unlink all STREAMs linked to the STREAM
				   associated with `fildes'.  */


/* Macros for `getmsg', `getpmsg', `putmsg' and `putpmsg'.  */
#define MSG_ANY		1	/* Receive any message.  */
#define MSG_BAND	2	/* Receive message from specified band.  */
#define MSG_HIPRI	3	/* Send/receive high priority message.  */
#define MORECTL		4	/* More control information is left in
				   message.  */
#define MOREDATA	5	/* More data is left in message.  */


/* Structure used for the I_FLUSHBAND ioctl on streams.  */
struct bandinfo
  {
    unsigned char bi_pri;
    int bi_flag;
  };

struct strbuf
  {
    int maxlen;		/* Maximum buffer length.  */
    int len;		/* Length of data.  */
    char *buf;		/* Pointer to buffer.  */
  };

struct strpeek
  {
    struct strbuf ctlbuf;
    struct strbuf databuf;
    __t_uscalar_t flags;
  };

struct strfdinsert
  {
    struct strbuf ctlbuf;
    struct strbuf databuf;
    __t_uscalar_t flags;
    int fildes;
    int offset;
  };

struct strioctl
  {
    int ic_cmd;
    int ic_timout;
    int ic_len;
    char *ic_dp;
  };

struct strrecvfd
  {
    int fd;
    __uid_t uid;
    __gid_t gid;
  };


struct str_mlist
  {
    char l_name[FMNAMESZ + 1];
  };

struct str_list
  {
    int sl_nmods;
    struct str_mlist *sl_modlist;
  };

#endif /* bits/stropts.h */
