# Makefile for support library, used only at build and test time
# Copyright (C) 2016-2017 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

subdir := support

include ../Makeconfig

extra-libs := libsupport
extra-libs-others = $(extra-libs)
extra-libs-noinstall := $(extra-libs)

libsupport-routines = \
  check \
  check_addrinfo \
  check_dns_packet \
  check_hostent \
  check_netent \
  delayed_exit \
  ignore_stderr \
  oom_error \
  resolv_test \
  set_fortify_handler \
  support_become_root \
  support_enter_network_namespace \
  support_format_address_family \
  support_format_addrinfo \
  support_format_dns_packet \
  support_format_herrno \
  support_format_hostent \
  support_format_netent \
  support_record_failure \
  support_run_diff \
  support_test_main \
  support_test_verify_impl \
  temp_file \
  write_message \
  xaccept \
  xasprintf \
  xbind \
  xcalloc \
  xconnect \
  xfclose \
  xfopen \
  xfork \
  xgetsockname \
  xlisten \
  xmalloc \
  xmemstream \
  xmmap \
  xmunmap \
  xpoll \
  xpthread_attr_destroy \
  xpthread_attr_init \
  xpthread_attr_setdetachstate \
  xpthread_attr_setstacksize \
  xpthread_barrier_destroy \
  xpthread_barrier_init \
  xpthread_barrier_wait \
  xpthread_cancel \
  xpthread_check_return \
  xpthread_cond_wait \
  xpthread_create \
  xpthread_detach \
  xpthread_join \
  xpthread_mutex_consistent \
  xpthread_mutex_destroy \
  xpthread_mutex_init \
  xpthread_mutex_lock \
  xpthread_mutex_unlock \
  xpthread_mutexattr_destroy \
  xpthread_mutexattr_init \
  xpthread_mutexattr_setprotocol \
  xpthread_mutexattr_setpshared \
  xpthread_mutexattr_setrobust \
  xpthread_mutexattr_settype \
  xpthread_once \
  xpthread_sigmask \
  xpthread_spin_lock \
  xpthread_spin_unlock \
  xrealloc \
  xrecvfrom \
  xsendto \
  xsetsockopt \
  xsocket \
  xstrdup \
  xwaitpid \
  xwrite \

libsupport-static-only-routines := $(libsupport-routines)
# Only build one variant of the library.
libsupport-inhibit-o := .os
ifeq ($(build-shared),yes)
libsupport-inhibit-o += .o
endif

tests = \
  README-testing \
  tst-support-namespace \
  tst-support_record_failure \

ifeq ($(run-built-tests),yes)
tests-special = \
  $(objpfx)tst-support_record_failure-2.out

$(objpfx)tst-support_record_failure-2.out: tst-support_record_failure-2.sh \
  $(objpfx)tst-support_record_failure
	$(SHELL) $< $(common-objpfx) '$(test-program-prefix-before-env)' \
	  '$(run-program-env)' '$(test-program-prefix-after-env)' \
	  > $@; \
	$(evaluate-test)
endif

include ../Rules
