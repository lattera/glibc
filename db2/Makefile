# Copyright (C) 1991, 92, 93, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

#
#	Sub-makefile for libdb.
#
#	The code is lifted straight from the db 2.3.12 distribution
#	with minimal changes.
#

subdir = db2

subdir-dirs = btree common db db185 dbm hash lock log mp mutex os txn \
	progs/db_archive progs/db_checkpoint  progs/db_deadlock \
	progs/db_dump progs/db_dump185 progs/db_load progs/db_printlog \
	progs/db_recover progs/db_stat clib

headers = db.h db_185.h

distribute = db_int.h config.h compat.h clib/getlong.c btree/btree.src \
	     db/db.src db185/db185_int.h hash/hash.src log/log.src \
	     txn/txn.src README \
	     $(addprefix include/,btree.h btree_auto.h btree_ext.h \
				  clib_ext.h common_ext.h cxx_int.h \
				  db.h.src db_185.h.src db_am.h db_auto.h \
				  db_cxx.h db_dispatch.h db_ext.h \
				  db_int.h.src db_page.h db_shash.h db_swap.h \
				  hash.h hash_auto.h hash_ext.h lock.h \
				  lock_ext.h log.h log_auto.h log_ext.h \
				  mp.h mp_ext.h mutex_ext.h os_ext.h queue.h \
				  shqueue.h txn.h txn_auto.h txn_ext.h \
				  os_func.h) \
	     $(addprefix mutex/,x86.gcc uts4.cc.s sparc.gcc parisc.hp \
				parisc.gcc alpha.gcc alpha.dec README \
				68020.gcc tsl_parisc.s)

vpath %.c $(subdir-dirs)

extra-libs := libdb
extra-libs-others := $(extra-libs)
extra-objs = getlong.o

libdb-routines := bt_close bt_compare bt_conv bt_cursor bt_delete \
	bt_open bt_page bt_put bt_rec bt_recno bt_rsearch bt_search \
	bt_split bt_stat btree_auto db db_appinit db_apprec \
	db_auto \
	db_byteorder db_conv db_dispatch db_dup db_err db_log2 os_alloc \
	os_abs os_config os_dir os_fid os_fsync os_map os_oflags \
	os_open os_rpath os_rw os_seek os_sleep os_stat os_unlink \
	os_spin db_overflow db_pr db_rec db_region db_ret db_salloc \
	db_shash db_thread hash hash_auto hash_conv hash_debug \
	hash_dup hash_func hash_page hash_rec hash_stat lock \
	lock_conflict lock_deadlock lock_util log log_archive \
	log_auto log_compare log_findckp log_get log_put log_rec \
	log_register mp_bh mp_fget mp_fopen mp_fput mp_fset \
	mp_open mp_pr mp_region mp_sync mutex txn txn_auto \
	txn_rec dbm db185

others		:= makedb db_dump185 db_archive db_checkpoint db_deadlock \
		db_dump db_load db_recover db_stat db_printlog
install-bin	:= makedb db_dump185 db_archive db_checkpoint db_deadlock \
		db_dump db_load db_recover db_stat db_printlog

include ../Rules

CPPFLAGS += -I./include -include ./compat.h

$(objpfx)db_checkpoint: $(objpfx)getlong.o
$(objpfx)db_deadlock: $(objpfx)getlong.o
$(objpfx)db_load: $(objpfx)getlong.o

ifeq ($(build-shared),yes)
$(objpfx)makedb: $(objpfx)libdb.so$(libdb.so-version)
$(objpfx)db_dump185: $(objpfx)libdb.so$(libdb.so-version)
$(objpfx)db_archive: $(objpfx)libdb.so$(libdb.so-version)
$(objpfx)db_checkpoint: $(objpfx)libdb.so$(libdb.so-version)
$(objpfx)db_deadlock: $(objpfx)libdb.so$(libdb.so-version)
$(objpfx)db_dump: $(objpfx)libdb.so$(libdb.so-version)
$(objpfx)db_load: $(objpfx)libdb.so$(libdb.so-version)
$(objpfx)db_printlog: $(objpfx)libdb.so$(libdb.so-version)
$(objpfx)db_recover: $(objpfx)libdb.so$(libdb.so-version)
$(objpfx)db_stat: $(objpfx)libdb.so$(libdb.so-version)
else
$(objpfx)makedb: $(objpfx)libdb.a
$(objpfx)db_dump185: $(objpfx)libdb.a
$(objpfx)db_archive: $(objpfx)libdb.a
$(objpfx)db_checkpoint: $(objpfx)libdb.a
$(objpfx)db_deadlock: $(objpfx)libdb.a
$(objpfx)db_dump: $(objpfx)libdb.a
$(objpfx)db_load: $(objpfx)libdb.a
$(objpfx)db_printlog: $(objpfx)libdb.a
$(objpfx)db_recover: $(objpfx)libdb.a
$(objpfx)db_stat: $(objpfx)libdb.a
endif

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
$(objpfx)libdb.so: $(common-objpfx)libc.so

subdir_install: $(inst_libdir)/libndbm.a
$(inst_libdir)/libndbm.a: $(inst_libdir)/libdb.a $(+force)
	$(make-link)

ifeq ($(build-shared),yes)
subdir_install: $(inst_libdir)/libndbm.so
$(inst_libdir)/libndbm.so: $(inst_libdir)/libdb.so $(+force)
	$(make-link)
endif
