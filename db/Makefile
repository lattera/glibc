# Makefile for 4.4BSD db code in GNU C library.
# This code is taken verbatim from the BSD db 1.85 package.  Only this
# Makefile and compat.h were written for GNU libc, and the header files
# moved up to this directory.

subdir = db

subdir-dirs = btree db hash mpool recno
vpath %.c $(subdir-dirs)

extra-libs := libdb1
extra-libs-others := $(extra-libs)
libdb1-routines := bt_close bt_conv bt_debug bt_delete bt_get \
		   bt_open bt_overflow bt_page bt_put bt_search \
		   bt_seq bt_split bt_utils \
		   db \
		   hash hash_bigkey hash_buf hash_func hash_log2 hash_page \
		   ndbm \
		   mpool \
		   rec_close rec_delete rec_get rec_open rec_put rec_search \
		   rec_seq rec_utils

db1-headers	:= db.h mpool.h ndbm.h
distribute	:= compat.h					\
		   btree/btree.h btree/extern.h			\
		   hash/extern.h hash/hash.h hash/page.h	\
		   recno/extern.h recno/recno.h			\
		   $(db1-headers)

include ../Makeconfig

install-others	:= $(inst_slibdir)/libdb.so$(libdb1.so-version) \
		   $(db1-headers:%=$(inst_includedir)/db1/%)

$(inst_slibdir)/libdb.so$(libdb1.so-version): $(inst_slibdir)/libdb1-$(version).so $(+force)
	rm -f $@
	$(LN_S) $(<F) $@

$(db1-headers:%=$(inst_includedir)/db1/%): $(inst_includedir)/db1/%: % $(+force)
	$(do-install)

include ../Rules

CPPFLAGS += -D__DBINTERFACE_PRIVATE -DUSE_LIBDB1

# This file defines some static functions for alternative hash algorithms
# that are not actually used.
CFLAGS-hash_func.c := -Wno-unused

# The db code outsmarts the compiler frequently.
override CFLAGS += -Wno-uninitialized

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libdb1.so: $(common-objpfx)libc.so

#subdir_install: $(inst_libdir)/libndbm.a
#$(inst_libdir)/libndbm.a: $(inst_libdir)/libdb1.a $(+force)
#	$(make-link)
#
#ifeq ($(build-shared),yes)
#subdir_install: $(inst_libdir)/libndbm.so
#$(inst_libdir)/libndbm.so: $(inst_libdir)/libdb1.so $(+force)
#	$(make-link)
#endif
