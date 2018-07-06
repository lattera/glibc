# Makefile for support library, used only at build and test time
# Copyright (C) 2016-2018 Free Software Foundation, Inc.
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
  next_to_fault \
  oom_error \
  resolv_test \
  set_fortify_handler \
  support-xfstat \
  support-xstat \
  support_become_root \
  support_can_chroot \
  support_capture_subprocess \
  support_capture_subprocess_check \
  support_chroot \
  support_descriptor_supports_holes \
  support_enter_mount_namespace \
  support_enter_network_namespace \
  support_format_address_family \
  support_format_addrinfo \
  support_format_dns_packet \
  support_format_herrno \
  support_format_hostent \
  support_format_netent \
  support_isolate_in_subprocess \
  support_openpty \
  support_paths \
  support_quote_blob \
  support_record_failure \
  support_run_diff \
  support_shared_allocate \
  support_test_compare_blob \
  support_test_compare_failure \
  support_write_file_string \
  support_test_main \
  support_test_verify_impl \
  temp_file \
  write_message \
  xaccept \
  xaccept4 \
  xasprintf \
  xbind \
  xcalloc \
  xchroot \
  xclose \
  xconnect \
  xdlfcn \
  xdup2 \
  xfclose \
  xfopen \
  xfork \
  xftruncate \
  xgetsockname \
  xlisten \
  xlseek \
  xmalloc \
  xmemstream \
  xmkdir \
  xmkdirp \
  xmmap \
  xmprotect \
  xmunmap \
  xopen \
  xpipe \
  xpoll \
  xpthread_attr_destroy \
  xpthread_attr_init \
  xpthread_attr_setdetachstate \
  xpthread_attr_setguardsize \
  xpthread_attr_setstacksize \
  xpthread_barrier_destroy \
  xpthread_barrier_init \
  xpthread_barrier_wait \
  xpthread_barrierattr_destroy \
  xpthread_barrierattr_init \
  xpthread_barrierattr_setpshared \
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
  xpthread_rwlock_init \
  xpthread_rwlock_rdlock \
  xpthread_rwlock_unlock \
  xpthread_rwlock_wrlock \
  xpthread_rwlockattr_init \
  xpthread_rwlockattr_setkind_np \
  xpthread_sigmask \
  xpthread_spin_lock \
  xpthread_spin_unlock \
  xraise \
  xreadlink \
  xrealloc \
  xrecvfrom \
  xsendto \
  xsetsockopt \
  xsigaction \
  xsignal \
  xsocket \
  xstrdup \
  xstrndup \
  xsymlink \
  xsysconf \
  xunlink \
  xwaitpid \
  xwrite \

libsupport-static-only-routines := $(libsupport-routines)
# Only build one variant of the library.
libsupport-inhibit-o := .os
ifeq ($(build-shared),yes)
libsupport-inhibit-o += .o
endif

CFLAGS-support_paths.c = \
		-DSRCDIR_PATH=\"`cd .. ; pwd`\" \
		-DOBJDIR_PATH=\"`cd $(objpfx)/..; pwd`\" \
		-DINSTDIR_PATH=\"$(prefix)\" \
		-DLIBDIR_PATH=\"$(libdir)\"

ifeq (,$(CXX))
LINKS_DSO_PROGRAM = links-dso-program-c
else
LINKS_DSO_PROGRAM = links-dso-program
LDLIBS-links-dso-program = -lstdc++ -lgcc_s
endif

others: \
	$(objpfx)test-container \
	$(objpfx)shell-container \
	$(objpfx)echo-container \
	$(objpfx)true-container \
	$(objpfx)$(LINKS_DSO_PROGRAM)

ifeq ($(build-programs),yes)
endif

LDLIBS-test-container = $(libsupport)

others += test-container
others-noinstall += test-container

others += shell-container echo-container true-container
others-noinstall += shell-container echo-container true-container

others += $(LINKS_DSO_PROGRAM)
others-noinstall += $(LINKS_DSO_PROGRAM)

$(objpfx)test-container : $(libsupport)
$(objpfx)shell-container : $(libsupport)
$(objpfx)echo-container : $(libsupport)
$(objpfx)true-container : $(libsupport)

tests = \
  README-testing \
  tst-support-namespace \
  tst-support_capture_subprocess \
  tst-support_format_dns_packet \
  tst-support_quote_blob \
  tst-support_record_failure \
  tst-test_compare \
  tst-test_compare_blob \
  tst-xreadlink \

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

$(objpfx)tst-support_format_dns_packet: $(common-objpfx)resolv/libresolv.so

include ../Rules
