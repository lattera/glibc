ifeq ($(subdir),nptl)
# We need to make sure that stack memory is allocated in the low 4GB.
CFLAGS-pthread_create.c += -DARCH_MAP_FLAGS=MAP_32BIT
endif
