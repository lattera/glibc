ifeq ($(subdir),math)
libm-support += fenv_const fe_nomask t_sqrt

# libm needs ld.so to access dl_hwcap
$(objpfx)libm.so: $(elfobjdir)/ld.so
endif
