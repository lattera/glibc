/* Copyright (C) 1993, 1994, 1995 Free Software Foundation, Inc.
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

#ifndef	_HURD_H

#define	_HURD_H	1
#include <features.h>


/* Get types, macros, constants and function declarations
   for all Mach microkernel interaction.  */
#include <mach.h>
#include <mach/mig_errors.h>

/* Get types and constants necessary for Hurd interfaces.  */
#include <hurd/hurd_types.h>

/* Get MiG stub declarations for commonly used Hurd interfaces.  */
#include <hurd/auth.h>
#include <hurd/process.h>
#include <hurd/fs.h>
#include <hurd/io.h>

/* Get `struct hurd_port' and related definitions implementing lightweight
   user references for ports.  These are used pervasively throughout the C
   library; this is here to avoid putting it in nearly every source file.  */
#include <hurd/port.h>

#include <errno.h>
#define	__hurd_fail(err)	(errno = (err), -1)

/* Basic ports and info, initialized by startup.  */

extern int _hurd_exec_flags;	/* Flags word passed in exec_startup.  */
extern struct hurd_port *_hurd_ports;
extern unsigned int _hurd_nports;
extern volatile mode_t _hurd_umask;

/* Shorthand macro for referencing _hurd_ports (see <hurd/port.h>).  */

#define	__USEPORT(which, expr) \
  HURD_PORT_USE (&_hurd_ports[INIT_PORT_##which], (expr))


/* Base address and size of the initial stack set up by the exec server.
   If using cthreads, this stack is deallocated in startup.
   Not locked.  */

extern vm_address_t _hurd_stack_base;
extern vm_size_t _hurd_stack_size;

/* Initial file descriptor table we were passed at startup.  If we are
   using a real dtable, these are turned into that and then cleared at
   startup.  If not, these are never changed after startup.  Not locked.  */

extern mach_port_t *_hurd_init_dtable;
extern mach_msg_type_number_t _hurd_init_dtablesize;

/* Current process IDs.  */

extern pid_t _hurd_pid, _hurd_ppid, _hurd_pgrp;
extern int _hurd_orphaned;

/* This variable is incremented every time the process IDs change.  */

unsigned int _hurd_pids_changed_stamp;

/* This condition is broadcast every time the process IDs change.  */
struct condition _hurd_pids_changed_sync;

/* Unix `data break', for brk and sbrk.
   If brk and sbrk are not used, this info will not be initialized or used.  */


/* Data break.  This is what `sbrk (0)' returns.  */

extern vm_address_t _hurd_brk;

/* End of allocated space.  This is generally `round_page (_hurd_brk)'.  */

extern vm_address_t _hurd_data_end;

/* This mutex locks _hurd_brk and _hurd_data_end.  */

extern struct mutex _hurd_brk_lock;

/* Set the data break to NEWBRK; _hurd_brk_lock must
   be held, and is released on return.  */

extern int _hurd_set_brk (vm_address_t newbrk);

#define __need_FILE
#include <stdio.h>

/* Calls to get and set basic ports.  */

extern error_t _hurd_ports_get (unsigned int which, mach_port_t *result);
extern error_t _hurd_ports_set (unsigned int which, mach_port_t newport);

extern process_t getproc (void);
extern file_t getcwdir (void), getcrdir (void);
extern auth_t getauth (void);
extern mach_port_t getcttyid ();
extern int setproc (process_t);
extern int setcwdir (file_t), setcrdir (file_t);
extern int setcttyid (mach_port_t);

/* Does reauth with the proc server and fd io servers.  */
extern int __setauth (auth_t), setauth (auth_t);


/* Split FILE into a directory and a name within the directory.  Look up a
   port for the directory and store it in *DIR; store in *NAME a pointer
   into FILE where the name within directory begins.  The directory lookup
   uses CRDIR for the root directory and CWDIR for the current directory.
   Returns zero on success or an error code.  */

extern error_t __hurd_file_name_split (file_t crdir, file_t cwdir,
				       const char *file,
				       file_t *dir, char **name);
extern error_t hurd_file_name_split (file_t crdir, file_t cwdir,
				     const char *file,
				     file_t *dir, char **name);

/* Open a port to FILE with the given FLAGS and MODE (see <fcntl.h>).
   The file lookup uses CRDIR for the root directory and CWDIR for the
   current directory.  If successful, returns zero and store the port
   to FILE in *PORT; otherwise returns an error code. */

extern error_t __hurd_file_name_lookup (file_t crdir, file_t cwdir,
					const char *file,
					int flags, mode_t mode,
					file_t *port);
extern error_t hurd_file_name_lookup (file_t crdir, file_t cwdir,
				      const char *filename,
				      int flags, mode_t mode,
				      file_t *port);

/* Process the values returned by `dir_lookup' et al, and loop doing
   `dir_lookup' calls until one returns FS_RETRY_NONE.  CRDIR is the
   root directory used for things like symlinks to absolute file names; the
   other arguments should be those just passed to and/or returned from
   `dir_lookup', `fsys_getroot', or `file_invoke_translator'.  This
   function consumes the reference in *RESULT even if it returns an error.  */

extern error_t __hurd_file_name_lookup_retry (file_t crdir,
					      enum retry_type doretry,
					      char retryname[1024],
					      int flags, mode_t mode,
					      file_t *result);
extern error_t hurd_file_name_lookup_retry (file_t crdir,
					    enum retry_type doretry,
					    char retryname[1024],
					    int flags, mode_t mode,
					    file_t *result);


/* Split FILE into a directory and a name within the directory.  The
   directory lookup uses the current root and working directory.  If
   successful, stores in *NAME a pointer into FILE where the name
   within directory begins and returns a port to the directory;
   otherwise sets `errno' and returns MACH_PORT_NULL.  */

extern file_t __file_name_split (const char *file, char **name);
extern file_t file_name_split (const char *file, char **name);

/* Open a port to FILE with the given FLAGS and MODE (see <fcntl.h>).
   The file lookup uses the current root and working directory.
   Returns a port to the file if successful; otherwise sets `errno'
   and returns MACH_PORT_NULL.  */

extern file_t __file_name_lookup (const char *file, int flags, mode_t mode);
extern file_t file_name_lookup (const char *file, int flags, mode_t mode);

/* Invoke any translator set on the node FILE represents, and return in
   *TRANSLATED a port to the translated node.  FLAGS are as for
   `dir_lookup' et al, but the returned port will not necessarily have
   any more access rights than FILE does.  */

extern error_t __hurd_invoke_translator (file_t file, int flags,
					 file_t *translated);
extern error_t hurd_invoke_translator (file_t file, int flags,
				       file_t *translated);


/* Open a file descriptor on a port.  FLAGS are as for `open'; flags
   affected by io_set_openmodes are not changed by this.  If successful,
   this consumes a user reference for PORT (which will be deallocated on
   close).  */

extern int openport (io_t port, int flags);

/* Open a stream on a port.  MODE is as for `fopen'.
   If successful, this consumes a user reference for PORT
   (which will be deallocated on fclose).  */

extern FILE *fopenport (io_t port, const char *mode);
extern FILE *__fopenport (io_t port, const char *mode);


/* Execute a file, replacing TASK's current program image.  */

extern error_t _hurd_exec (task_t task,
			   file_t file,
			   char *const argv[],
			   char *const envp[]);


/* Inform the proc server we have exitted with STATUS, and kill the
   task thoroughly.  This function never returns, no matter what.  */

extern void _hurd_exit (int status) __attribute__ ((noreturn));


/* Initialize Mach RPCs and essential Hurd things (_hurd_preinit_hook); do
   initial handshake with the exec server (or extract the arguments from
   the stack in the case of the bootstrap task); if cthreads is in use,
   initialize it now and switch the calling thread to a cthread stack;
   finally, call *MAIN with the information gleaned.  That function is not
   expected to return.  ARGPTR should be the address of the first argument
   of the entry point function that is called with the stack exactly as the
   exec server or kernel sets it.  */

extern void _hurd_startup (void **argptr,
			   void (*main) (int argc, char **argv, char **envp,
					 mach_port_t *portarray,
					 mach_msg_type_number_t portarraysize,
					 int *intarray,
					 mach_msg_type_number_t intarraysize))
     __attribute__ ((noreturn));

/* Initialize the library data structures from the
   ints and ports passed to us by the exec server.
   Then vm_deallocate PORTARRAY and INTARRAY.  */

extern void _hurd_init (int flags, char **argv,
			mach_port_t *portarray, size_t portarraysize,
			int *intarray, size_t intarraysize);

/* Do startup handshaking with the proc server.  */

extern void _hurd_proc_init (char **argv);


/* Return the socket server for sockaddr domain DOMAIN.  If DEAD is
   nonzero, remove the old cached port and always do a fresh lookup.

   It is assumed that a socket server will stay alive during a complex socket
   operation involving several RPCs.  But a socket server may die during
   long idle periods between socket operations.  Callers should first pass
   zero for DEAD; if the first socket RPC tried on the returned port fails
   with MACH_SEND_INVALID_DEST or MIG_SERVER_DIED (indicating the server
   went away), the caller should call _hurd_socket_server again with DEAD
   nonzero and retry the RPC on the new socket server port.  */

extern socket_t _hurd_socket_server (int domain, int dead);

/* Send a `sig_post' RPC to process number PID.  If PID is zero,
   send the message to all processes in the current process's process group.
   If PID is < -1, send SIG to all processes in process group - PID.
   SIG and REFPORT are passed along in the request message.  */

extern error_t _hurd_sig_post (pid_t pid, int sig, mach_port_t refport);
extern error_t hurd_sig_post (pid_t pid, int sig, mach_port_t refport);

/* Fetch the host privileged port and device master port from the proc
   server.  They are fetched only once and then cached in the
   variables below.  A special program that gets them from somewhere
   other than the proc server (such as a bootstrap filesystem) can set
   these variables to install the ports.  */

extern kern_return_t get_privileged_ports (host_priv_t *host_priv_ptr,
					   device_t *device_master_ptr);
extern mach_port_t _hurd_host_priv, _hurd_device_master;

/* Return the PID of the task whose control port is TASK.
   On error, sets `errno' and returns -1.  */

extern pid_t __task2pid (task_t task), task2pid (task_t task);

/* Return the task control port of process PID.
   On error, sets `errno' and returns MACH_PORT_NULL.  */

extern task_t __pid2task (pid_t pid), pid2task (pid_t pid);


/* Return the io server port for file descriptor FD.
   This adds a Mach user reference to the returned port.
   On error, sets `errno' and returns MACH_PORT_NULL.  */

extern io_t __getdport (int fd), getdport (int fd);


#endif	/* hurd.h */
