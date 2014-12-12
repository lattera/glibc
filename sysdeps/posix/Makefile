# These affect the generated bits/stdio_lim.h file.
L_tmpnam  = 20
TMP_MAX   = 238328
L_ctermid = 9
L_cuserid = 9

ifeq ($(subdir)|$(have-thread-library),rt|no)
# With NPTL, this lives in libpthread so it can be used for sem_open too.
# Without NPTL, it's just private in librt.
librt-routines += shm-directory
endif
