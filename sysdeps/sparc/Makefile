# The Sparc `long double' is a distinct type we support.
long-double-fcts = yes

pie-ccflag = -fPIE

ifeq ($(subdir),gmon)
sysdep_routines += sparc-mcount
endif

ifeq ($(subdir),db2)
CPPFLAGS += -DHAVE_SPINLOCKS=1 -DHAVE_ASSEM_SPARC_GCC=1
endif

ifeq ($(subdir),csu)
# get offset to rtld_global._dl_hwcap
gen-as-const-headers += rtld-global-offsets.sym
endif
