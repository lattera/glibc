ifeq ($(subdir),signal)
#sysdep_routines += sigsuspend
endif

ifeq ($(subdir),misc)
sysdep_routines += cachectl cacheflush sysmips _test_and_set

sysdep_headers += sys/cachectl.h sys/sysmips.h sys/tas.h

no_syscall_list_h = 1

# Generate the list of SYS_* macros for the system calls (__NR_* macros).
# We generate not only SYS_<syscall>, pointing at SYS_<abi>_<syscall> if
# it exists, but also define SYS_<abi>_<syscall> for all ABIs.
$(objpfx)syscall-%.h $(objpfx)syscall-%.d: ../sysdeps/unix/sysv/linux/mips/sys/syscall.h
	$(make-target-directory)
	{ \
	 echo '/* Generated at libc build time from kernel syscall list.  */';\
	 echo ''; \
	 echo '#ifndef _SYSCALL_H'; \
	 echo '# error "Never use <bits/syscall.h> directly; include <sys/syscall.h> instead."'; \
	 echo '#endif'; \
	 echo ''; \
	 echo '#include <sgidefs.h>'; \
	 rm -f $(@:.d=.h).newt; \
	 $(CC) -E -MD -MP -MF $(@:.h=.d)-t -MT '$(@:.d=.h) $(@:.h=.d)' \
	       -x c -I $(common-objdir) $(sysincludes) $< -D_LIBC -dM | \
	 sed -n 's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p' \
	     > $(@:.d=.h).newt; \
	 if grep SYS_O32_ $(@:.d=.h).newt > /dev/null; then \
	   echo '#if _MIPS_SIM == _MIPS_SIM_ABI64'; \
	   sed -n 's/^\(#define SYS_\)N64_/\1/p' < $(@:.d=.h).newt; \
	   echo '#elif _MIPS_SIM == _MIPS_SIM_NABI32'; \
	   sed -n 's/^\(#define SYS_\)N32_/\1/p' < $(@:.d=.h).newt; \
	   echo '#else'; \
	   sed -n 's/^\(#define SYS_\)O32_/\1/p' < $(@:.d=.h).newt; \
	   echo '#endif'; \
	   sed -n '/^#define SYS_\([ON]32\|N64\)_/p' < $(@:.d=.h).newt; \
	 else \
	   cat $(@:.d=.h).newt; \
	 fi; \
	 rm $(@:.d=.h).newt; \
	} > $(@:.d=.h).new
	mv -f $(@:.d=.h).new $(@:.d=.h)
ifneq (,$(objpfx))
	sed $(sed-remove-objpfx) $(@:.h=.d)-t > $(@:.h=.d)-t2
	rm -f $(@:.h=.d)-t
	mv -f $(@:.h=.d)-t2 $(@:.h=.d)
else
	mv -f $(@:.h=.d)-t $(@:.h=.d)
endif
endif
