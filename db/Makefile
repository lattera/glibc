# Makefile for 4.4BSD db code in GNU C library.
# This code is taken verbatim from the BSD db 1.85 package.  Only this
# Makefile and compat.h were written for GNU libc, and the header files
# moved up to this directory.

subdir = db

subdir-dirs = btree db hash mpool recno
vpath %.c $(subdir-dirs)

routines := bt_close bt_conv bt_debug bt_delete bt_get bt_open bt_overflow \
	    bt_page bt_put bt_search bt_seq bt_split bt_utils		   \
	    db								   \
	    hash hash_bigkey hash_buf hash_func hash_log2 hash_page	   \
	    ndbm							   \
	    mpool							   \
	    rec_close rec_delete rec_get rec_open rec_put rec_search	   \
	    rec_seq rec_utils

headers		:= db.h mpool.h ndbm.h
distribute	:= compat.h					\
		   btree/btree.h btree/extern.h			\
		   hash/extern.h hash/hash.h hash/page.h	\
		   recno/extern.h recno/recno.h

include ../Rules

CPPFLAGS += -D__DBINTERFACE_PRIVATE
