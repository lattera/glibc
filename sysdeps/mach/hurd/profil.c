/* Low-level statistical profiling support function.  Mach/Hurd version.
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

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <hurd.h>
#include <mach/mach4.h>
#include <mach/pc_sample.h>
#include <cthreads.h>
#include <assert.h>

static thread_t profile_thread = MACH_PORT_NULL;
static u_short *samples;
static size_t maxsamples;
static size_t pc_offset;
static size_t sample_scale;
static sampled_pc_seqno_t seqno;
static struct mutex lock = MUTEX_INITIALIZER;

/* Enable statistical profiling, writing samples of the PC into at most
   SIZE bytes of SAMPLE_BUFFER; every processor clock tick while profiling
   is enabled, the system examines the user PC and increments
   SAMPLE_BUFFER[((PC - OFFSET) / 2) * SCALE / 65536].  If SCALE is zero,
   disable profiling.  Returns zero on success, -1 on error.  */

static error_t
update_waiter (u_short *sample_buffer, size_t size, size_t offset, u_int scale)
{
  error_t err;

  if (profile_thread == MACH_PORT_NULL)
    {
      /* Set up the profiling collector thread.  */
      static void profile_waiter (void);
      err = __thread_create (__mach_task_self (), &profile_thread);
      if (! err)
	err = __mach_setup_thread (__mach_task_self (), profile_thread,
				   &profile_waiter, NULL, NULL);
    }
  else
    err = 0;

  if (! err)
    {
      if (sample_scale == 0)
	err = __thread_resume (profile_thread);
      if (! err)
	{
	  samples = sample_buffer;
	  maxsamples = size / sizeof *sample_buffer;
	  pc_offset = offset;
	  sample_scale = scale;
	}
    }

  return err;
}

int
profil (u_short *sample_buffer, size_t size, size_t offset, u_int scale)
{
  error_t err;

  __mutex_lock (&lock);

  if (scale == 0)
    {
      /* Disable profiling.  */
      int count;
      __thread_suspend (profile_thread);
      err = __task_disable_pc_sampling (__mach_task_self (), &count);
      sample_scale = 0;
      seqno = 0;
    }
  else
    err = update_waiter (sample_buffer, size, offset, scale);

  __mutex_unlock (&lock);

  return err ? __hurd_fail (err) : 0;
}

static void
profile_waiter (void)
{
  sampled_pc_t pc_samples[512];
  mach_msg_type_number_t nsamples = 512, i;
  mach_port_t rcv = __mach_reply_port ();
  mach_msg_header_t msg;
  const mach_msg_timeout_t timeout = 17; /* ??? XXX */
  error_t err;

  while (1)
    {
      __mutex_lock (&lock);

      err = __task_get_sampled_pcs (__mach_task_self (), &seqno,
				    pc_samples, &nsamples);
      assert_perror (err);

      for (i = 0; i < nsamples; ++i)
	{
	  size_t idx = (((pc_samples[i].pc - pc_offset) / 2) *
			sample_scale / 65536);
	  if (idx < maxsamples)
	    ++samples[idx];
	}

      __vm_deallocate (__mach_task_self (),
		       (vm_address_t) pc_samples,
		       nsamples * sizeof *pc_samples);

      __mutex_unlock (&lock);

      __mach_msg (&msg, MACH_RCV_MSG|MACH_RCV_TIMEOUT, 0, sizeof msg,
		  rcv, timeout, MACH_PORT_NULL);
    }
}

data_set_element (_hurd_fork_locks, lock);

static void
fork_profil (void)
{
  u_short *sb;
  size_t n, o, ss;
  error_t err;

  if (profile_thread != MACH_PORT_NULL)
    {
      __mach_port_deallocate (__mach_task_self (), profile_thread);
      profile_thread = MACH_PORT_NULL;
    }

  sb = samples;
  samples = NULL;
  n = maxsamples;
  maxsamples = 0;
  o = pc_offset;
  pc_offset = 0;
  ss = sample_scale;
  sample_scale = 0;

  err = update_waiter (sb, n * sizeof *sb, o, ss);
  assert_perror (err);
}
text_set_element (_hurd_fork_child_hook, fork_profil);
