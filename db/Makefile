# Makefile for 4.4BSD db code in GNU C library.
# This code is taken verbatim from the BSD db 1.85 package.  Only this
# Makefile and compat.h were written for GNU libc, and the header files
# moved up to this directory.

subdir = db

subdir-dirs = btree db hash mpool recno
vpath %.c $(subdir-dirs)

extra-libs := libdb
extra-libs-others := $(extra-libs)
libdb-routines := bt_close bt_conv bt_debug bt_delete bt_get \
		  bt_open bt_overflow bt_page bt_put bt_search \
		  bt_seq bt_split bt_utils \
		  db \
		  hash hash_bigkey hash_buf hash_func hash_log2 hash_page \
		  ndbm \
		  mpool \
		  rec_close rec_delete rec_get rec_open rec_put rec_search \
		  rec_seq rec_utils
libdb-map	:= libdb.map

headers		:= db.h mpool.h ndbm.h
distribute	:= compat.h					\
		   btree/btree.h btree/extern.h			\
		   hash/extern.h hash/hash.h hash/page.h	\
		   recno/extern.h recno/recno.h

others		:= makedb
install-bin	:= makedb

include ../Rules

CPPFLAGS += -D__DBINTERFACE_PRIVATE

# This file defines some static functions for alternative hash algorithms
# that are not actually used.
CFLAGS-hash_func.c := -Wno-unused

# The db code outsmarts the compiler frequently.
override CFLAGS += -Wno-uninitialized

ifeq ($(build-shared),yes)
$(objpfx)makedb: $(objpfx)libdb.so$(libdb.so-version)
else
$(objpfx)makedb: $(objpfx)libdb.a
endif

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libdb.so: $(common-objpfx)libc.so

subdir_install: $(inst_libdir)/libndbm.a
$(inst_libdir)/libndbm.a: $(inst_libdir)/libdb.a $(+force)
	$(make-link)

ifeq ($(build-shared),yes)
subdir_install: $(inst_libdir)/libndbm.so
$(inst_libdir)/libndbm.so: $(inst_libdir)/libdb.so $(+force)
	$(make-link)
endif
