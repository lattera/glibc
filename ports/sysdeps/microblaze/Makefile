ifeq ($(subdir),elf)
CFLAGS-rtld.c += -Wno-uninitialized -Wno-unused
endif

ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif

ifeq ($(subdir),resource)
sysdep_routines += backtrace_linux
endif

$(objpfx)libm.so: $(elfobjdir)/ld.so
$(objpfx)libcrypt.so: $(elfobjdir)/ld.so
$(objpfx)libresolv.so: $(elfobjdir)/ld.so
$(objpfx)libnss_dns.so: $(elfobjdir)/ld.so
$(objpfx)libnss_files.so: $(elfobjdir)/ld.so
$(objpfx)libnss_db.so: $(elfobjdir)/ld.so
$(objpfx)libnss_nis.so: $(elfobjdir)/ld.so
$(objpfx)libnss_nisplus.so: $(elfobjdir)/ld.so
$(objpfx)libnss_hesiod.so: $(elfobjdir)/ld.so
$(objpfx)libnss_compat.so: $(elfobjdir)/ld.so
$(objpfx)libanl.so: $(elfobjdir)/ld.so
$(objpfx)libnsl.so: $(elfobjdir)/ld.so
$(objpfx)libcidn.so: $(elfobjdir)/ld.so
$(objpfx)libutil.so: $(elfobjdir)/ld.so
