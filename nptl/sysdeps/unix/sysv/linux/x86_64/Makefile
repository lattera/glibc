ifeq ($(subdir),nptl)
CFLAGS-pt-initfini.s = -g0 -fPIC -fno-inline-functions -fno-asynchronous-unwind-tables

# We need to make sure that stack memory is allocated in the low 4GB.
CFLAGS-pthread_create.c += -DARCH_MAP_FLAGS=MAP_32BIT
endif
