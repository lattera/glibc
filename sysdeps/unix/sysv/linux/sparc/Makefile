ifeq ($(subdir),misc)

no_syscall_list_h = 1

# Generate the list of SYS_* macros for the system calls (__NR_* macros).
$(objpfx)syscall-%.h $(objpfx)syscall-%.d: ../sysdeps/unix/sysv/linux/sys/syscall.h
	rm -f $(@:.h=.d)-t
	{ \
	 echo '/* Generated at libc build time from kernel syscall list.  */';\
	 echo ''; \
	 echo '#ifndef _SYSCALL_H'; \
	 echo '# error "Never use <bits/syscall.h> directly; include <sys/syscall.h> instead."'; \
	 echo '#endif'; \
	 echo ''; \
	 SUNPRO_DEPENDENCIES='$(@:.h=.d)-t $@' \
	 $(CC) -E -x c $(sysincludes) $< -U__sparc_v9__ -U__arch64__ -D_LIBC -dM | \
	 sed -n 's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p' | \
	 LC_ALL=C sort > $(@:.d=.h).new32; \
	 SUNPRO_DEPENDENCIES='$(@:.h=.d)-t $@' \
	 $(CC) -E -x c $(sysincludes) $< -D__sparc_v9__ -D__arch64__ -D_LIBC -dM | \
	 sed -n 's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p' | \
	 LC_ALL=C sort > $(@:.d=.h).new64; \
	 if cmp -s $(@:.d=.h).new32 $(@:.d=.h).new64; then \
	   cat $(@:.d=.h).new32; \
	 else \
	   echo '#include <bits/wordsize.h>'; \
	   echo ''; \
	   comm -12 $(@:.d=.h).new32 $(@:.d=.h).new64; \
	   echo '#if __WORDSIZE == 64'; \
	   comm -13 $(@:.d=.h).new32 $(@:.d=.h).new64; \
	   echo '#else'; \
	   comm -23 $(@:.d=.h).new32 $(@:.d=.h).new64; \
	   echo '#endif'; \
	 fi; \
	 rm -f $(@:.d=.h).new32 $(@:.d=.h).new64; \
	} > $(@:.d=.h).new
	mv -f $(@:.d=.h).new $(@:.d=.h)
	sed < $(@:.h=.d)-t > $(@:.h=.d)-t2 \
	    -e 's,$(subst .,\.,$@),$(patsubst $(objpfx)%,$$(objpfx)%,\
					      $(@:.d=.h) $(@:.h=.d)),'
	rm -f $(@:.h=.d)-t
	mv -f $(@:.h=.d)-t2 $(@:.h=.d)

endif

ifeq ($(subdir),elf)
ifeq (yes,$(build-shared))
# This is needed to support g++ v2 and v3.
sysdep_routines += framestate
shared-only-routines += framestate
endif
endif
