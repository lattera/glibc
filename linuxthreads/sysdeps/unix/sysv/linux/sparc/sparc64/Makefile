ifeq ($(subdir),linuxthreads)
# gcc -m64 has /usr/lib64/crti.o hardcoded in the specs file, because otherwise
# it would normally find 32bit crti.o.
LDFLAGS-pthread.so += -specs=$(objpfx)specs
before-compile += $(objpfx)specs
postclean-generated += $(subdir)specs
$(objpfx)specs:
	$(CC) $(CFLAGS) $(CPPFLAGS) -dumpspecs | sed 's_/usr/lib64/crti.o_crti.o_g' > $@
endif
