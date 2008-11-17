ifeq ($(subdir),math)
libm-support += fenv_const fe_nomask fe_mask t_sqrt
libm-tests += test-powerpc-snan

# libm needs ld.so to access dl_hwcap
$(objpfx)libm.so: $(elfobjdir)/ld.so
endif

ifeq ($(subdir),stdlib)
tests += tst-setcontext-fpscr
endif
