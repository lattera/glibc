ifeq ($(subdir),linuxthreads)
CFLAGS-pt-initfini.s = -g0 -fPIC -fno-inline-functions -fno-asynchronous-unwind-tables
# gcc -m64 has /usr/lib64/crti.o hardcoded in the specs file, because otherwise
# it would normally find 32bit crti.o.
LDFLAGS-pthread.so += -specs=$(objpfx)specs
before-compile += $(objpfx)specs
generated += specs
$(objpfx)specs:
	$(CC) $(CFLAGS) $(CPPFLAGS) -dumpspecs \
	| sed 's,\(/usr/lib\(\|64\)/\|^\|\([^/a-z]\)\)\(crt[in]\.o\),\3./\4,g' > $@.new
	mv -f $@.new $@
endif
