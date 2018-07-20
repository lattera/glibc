# Copyright (C) 1995-2018 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

# Makefile for elf subdirectory of GNU C Library.

subdir		:= elf

include ../Makeconfig

headers		= elf.h bits/elfclass.h link.h bits/link.h
routines	= $(all-dl-routines) dl-support dl-iteratephdr \
		  dl-addr dl-addr-obj enbl-secure dl-profstub \
		  dl-origin dl-libc dl-sym dl-sysdep dl-error \
		  dl-reloc-static-pie

# The core dynamic linking functions are in libc for the static and
# profiled libraries.
dl-routines	= $(addprefix dl-,load lookup object reloc deps hwcaps \
				  runtime init fini debug misc \
				  version profile tls origin scope \
				  execstack open close trampoline \
				  exception sort-maps)
ifeq (yes,$(use-ldconfig))
dl-routines += dl-cache
endif

ifneq (no,$(have-tunables))
dl-routines += dl-tunables
tunables-type = $(addprefix TUNABLES_FRONTEND_,$(have-tunables))
CPPFLAGS-dl-tunables.c += -DTUNABLES_FRONTEND=$(tunables-type)

# Make sure that the compiler does not insert any library calls in tunables
# code paths.
ifeq (yes,$(have-loop-to-function))
CFLAGS-dl-tunables.c += -fno-tree-loop-distribute-patterns
endif
endif

all-dl-routines = $(dl-routines) $(sysdep-dl-routines)
# But they are absent from the shared libc, because that code is in ld.so.
elide-routines.os = $(all-dl-routines) dl-support enbl-secure dl-origin \
		    dl-sysdep dl-exception dl-reloc-static-pie

# ld.so uses those routines, plus some special stuff for being the program
# interpreter and operating independent of libc.
rtld-routines	= rtld $(all-dl-routines) dl-sysdep dl-environ dl-minimal \
  dl-error-minimal dl-conflict
all-rtld-routines = $(rtld-routines) $(sysdep-rtld-routines)

CFLAGS-dl-runtime.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-dl-lookup.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-dl-iterate-phdr.c += $(uses-callbacks)

# Compile rtld itself without stack protection.
# Also compile all routines in the static library that are elided from
# the shared libc because they are in libc.a in the same way.

define elide-stack-protector
$(if $(filter $(@F),$(patsubst %,%$(1),$(2))), $(no-stack-protector))
endef

CFLAGS-.o += $(call elide-stack-protector,.o,$(elide-routines.os))
CFLAGS-.op += $(call elide-stack-protector,.op,$(elide-routines.os))
CFLAGS-.os += $(call elide-stack-protector,.os,$(all-rtld-routines))

ifeq ($(unwind-find-fde),yes)
routines += unwind-dw2-fde-glibc
shared-only-routines += unwind-dw2-fde-glibc
endif

before-compile  += $(objpfx)trusted-dirs.h
generated	+= trusted-dirs.h trusted-dirs.st for-renamed/renamed.so
generated-dirs	+= for-renamed

ifeq ($(build-shared),yes)
ld-map		= $(common-objpfx)ld.map
endif

ifeq (yes,$(build-shared))
extra-objs	= $(all-rtld-routines:%=%.os) soinit.os sofini.os interp.os
generated	+= librtld.os dl-allobjs.os ld.so ldd
install-others	= $(inst_rtlddir)/$(rtld-installed-name)
install-bin-script = ldd
endif

others		= sprof sln
install-bin	= sprof
others-static   = sln
install-rootsbin = sln
sln-modules	:= static-stubs
extra-objs	+= $(sln-modules:=.o)

ifeq (yes,$(use-ldconfig))
ifeq (yes,$(build-shared))
others-static	+= ldconfig
others		+= ldconfig
install-rootsbin += ldconfig

ldconfig-modules := cache readlib xmalloc xstrdup chroot_canon static-stubs
extra-objs	+= $(ldconfig-modules:=.o)
others-extras   = $(ldconfig-modules)
endif
endif

# To find xmalloc.c and xstrdup.c
vpath %.c ../locale/programs

ifeq ($(build-shared),yes)
extra-objs += sotruss-lib.os sotruss-lib.so
install-others += $(inst_auditdir)/sotruss-lib.so
install-bin-script += sotruss
generated += sotruss
libof-sotruss-lib = extramodules
$(objpfx)sotruss-lib.so: $(objpfx)sotruss-lib.os
	$(build-module-asneeded)
$(objpfx)sotruss-lib.so: $(common-objpfx)libc.so $(objpfx)ld.so \
	$(common-objpfx)libc_nonshared.a

$(objpfx)sotruss: sotruss.sh $(common-objpfx)config.make
	sed -e 's%@BASH@%$(BASH)%g' \
	    -e 's%@VERSION@%$(version)%g' \
	    -e 's%@TEXTDOMAINDIR@%$(localedir)%g' \
	    -e 's%@PREFIX@%$(prefix)%g' \
	    -e 's|@PKGVERSION@|$(PKGVERSION)|g' \
	    -e 's|@REPORT_BUGS_TO@|$(REPORT_BUGS_TO)|g' \
	    < $< > $@.new
	chmod 555 $@.new
	mv -f $@.new $@
$(inst_auditdir)/sotruss-lib.so: $(objpfx)sotruss-lib.so $(+force)
	$(do-install-program)
endif

tests-static-normal := tst-leaks1-static tst-array1-static tst-array5-static \
	       tst-dl-iter-static \
	       tst-tlsalign-static tst-tlsalign-extern-static \
	       tst-linkall-static tst-env-setuid tst-env-setuid-tunables
tests-static-internal := tst-tls1-static tst-tls2-static \
	       tst-ptrguard1-static tst-stackguard1-static \
	       tst-tls1-static-non-pie tst-libc_dlvsym-static

CRT-tst-tls1-static-non-pie := $(csu-objpfx)crt1.o
tst-tls1-static-non-pie-no-pie = yes

tests := tst-tls9 tst-leaks1 \
	tst-array1 tst-array2 tst-array3 tst-array4 tst-array5 \
	tst-auxv
tests-internal := tst-tls1 tst-tls2 $(tests-static-internal)
tests-static := $(tests-static-normal) $(tests-static-internal)

ifeq (yes,$(build-shared))
tests-static += tst-tls9-static
tst-tls9-static-ENV = \
       LD_LIBRARY_PATH=$(objpfx):$(common-objpfx):$(common-objpfx)dlfcn

tests += restest1 preloadtest loadfail multiload origtest resolvfail \
	 constload1 order noload filter \
	 reldep reldep2 reldep3 reldep4 nodelete nodelete2 \
	 nodlopen nodlopen2 lateglobal initfirst global \
	 restest2 next dblload dblunload reldep5 reldep6 reldep7 reldep8 \
	 tst-tls4 tst-tls5 \
	 tst-tls10 tst-tls11 tst-tls12 tst-tls13 tst-tls14 tst-tls15 \
	 tst-tls16 tst-tls17 tst-tls18 tst-tls19 tst-tls-dlinfo \
	 tst-align tst-align2 \
	 tst-dlmodcount tst-dlopenrpath tst-deep1 \
	 tst-dlmopen1 tst-dlmopen3 \
	 unload3 unload4 unload5 unload6 unload7 unload8 tst-global1 order2 \
	 tst-audit1 tst-audit2 tst-audit8 tst-audit9 \
	 tst-addr1 tst-thrlock \
	 tst-unique1 tst-unique2 $(if $(CXX),tst-unique3 tst-unique4 \
	 tst-nodelete) \
	 tst-initorder tst-initorder2 tst-relsort1 tst-null-argv \
	 tst-tlsalign tst-tlsalign-extern tst-nodelete-opened \
	 tst-nodelete2 tst-audit11 tst-audit12 tst-dlsym-error tst-noload \
	 tst-latepthread tst-tls-manydynamic tst-nodelete-dlclose \
	 tst-debug1 tst-main1 tst-absolute-sym tst-absolute-zero tst-big-note
#	 reldep9
tests-internal += loadtest unload unload2 circleload1 \
	 neededtest neededtest2 neededtest3 neededtest4 \
	 tst-tls3 tst-tls6 tst-tls7 tst-tls8 tst-dlmopen2 \
	 tst-ptrguard1 tst-stackguard1 tst-libc_dlvsym
ifeq ($(build-hardcoded-path-in-tests),yes)
tests += tst-dlopen-aout
tst-dlopen-aout-no-pie = yes
endif
test-srcs = tst-pathopt
selinux-enabled := $(shell cat /selinux/enforce 2> /dev/null)
ifneq ($(selinux-enabled),1)
tests-execstack-yes = tst-execstack tst-execstack-needed tst-execstack-prog
endif
endif
tests += $(tests-execstack-$(have-z-execstack))
ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)tst-leaks1-mem.out \
		 $(objpfx)tst-leaks1-static-mem.out $(objpfx)noload-mem.out \
		 $(objpfx)tst-ldconfig-X.out
endif
tlsmod17a-suffixes = 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
tlsmod18a-suffixes = 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
tlsmod17a-modules = $(addprefix tst-tlsmod17a, $(tlsmod17a-suffixes))
tlsmod18a-modules = $(addprefix tst-tlsmod18a, $(tlsmod17a-suffixes))
one-hundred = $(foreach x,0 1 2 3 4 5 6 7 8 9, \
  0$x 1$x 2$x 3$x 4$x 5$x 6$x 7$x 8$x 9$x)
tst-tls-many-dynamic-modules := \
  $(foreach n,$(one-hundred),tst-tls-manydynamic$(n)mod)
extra-test-objs += $(tlsmod17a-modules:=.os) $(tlsmod18a-modules:=.os) \
		   tst-tlsalign-vars.o
test-extras += tst-tlsmod17a tst-tlsmod18a tst-tlsalign-vars
modules-names = testobj1 testobj2 testobj3 testobj4 testobj5 testobj6 \
		testobj1_1 failobj constload2 constload3 unloadmod \
		dep1 dep2 dep3 dep4 vismod1 vismod2 vismod3 \
		nodelmod1 nodelmod2 nodelmod3 nodelmod4 \
		nodel2mod1 nodel2mod2 nodel2mod3 \
		nodlopenmod nodlopenmod2 filtmod1 filtmod2 \
		reldepmod1 reldepmod2 reldepmod3 reldepmod4 nextmod1 nextmod2 \
		reldep4mod1 reldep4mod2 reldep4mod3 reldep4mod4 \
		neededobj1 neededobj2 neededobj3 neededobj4 \
		neededobj5 neededobj6 firstobj globalmod1 \
		unload2mod unload2dep ltglobmod1 ltglobmod2 pathoptobj \
		dblloadmod1 dblloadmod2 dblloadmod3 reldepmod5 reldepmod6 \
		reldep6mod0 reldep6mod1 reldep6mod2 reldep6mod3 reldep6mod4 \
		reldep7mod1 reldep7mod2 \
		tst-tlsmod1 tst-tlsmod2 tst-tlsmod3 tst-tlsmod4 \
		tst-tlsmod5 tst-tlsmod6 tst-tlsmod7 tst-tlsmod8 \
		tst-tlsmod9 tst-tlsmod10 tst-tlsmod11 tst-tlsmod12 \
		tst-tlsmod13 tst-tlsmod13a tst-tlsmod14a tst-tlsmod14b \
		tst-tlsmod15a tst-tlsmod15b tst-tlsmod16a tst-tlsmod16b \
		$(tlsmod17a-modules) tst-tlsmod17b $(tlsmod18a-modules) \
		tst-tls19mod1 tst-tls19mod2 tst-tls19mod3 \
		circlemod1 circlemod1a circlemod2 circlemod2a \
		circlemod3 circlemod3a \
		reldep8mod1 reldep8mod2 reldep8mod3 \
		reldep9mod1 reldep9mod2 reldep9mod3 \
		tst-alignmod tst-alignmod2 \
		$(modules-execstack-$(have-z-execstack)) \
		tst-dlopenrpathmod tst-deep1mod1 tst-deep1mod2 tst-deep1mod3 \
		tst-dlmopen1mod tst-auditmod1 \
		unload3mod1 unload3mod2 unload3mod3 unload3mod4 \
		unload4mod1 unload4mod2 unload4mod3 unload4mod4 \
		unload6mod1 unload6mod2 unload6mod3 \
		unload7mod1 unload7mod2 \
		unload8mod1 unload8mod1x unload8mod2 unload8mod3 \
		order2mod1 order2mod2 order2mod3 order2mod4 \
		tst-unique1mod1 tst-unique1mod2 \
		tst-unique2mod1 tst-unique2mod2 \
		tst-auditmod9a tst-auditmod9b \
		$(if $(CXX),tst-unique3lib tst-unique3lib2 tst-unique4lib \
		  tst-nodelete-uniquemod tst-nodelete-rtldmod \
		  tst-nodelete-zmod) \
		tst-initordera1 tst-initorderb1 \
		tst-initordera2 tst-initorderb2 \
		tst-initordera3 tst-initordera4 \
		tst-initorder2a tst-initorder2b tst-initorder2c \
		tst-initorder2d \
		tst-relsort1mod1 tst-relsort1mod2 tst-array2dep \
		tst-array5dep tst-null-argv-lib \
		tst-tlsalign-lib tst-nodelete-opened-lib tst-nodelete2mod \
		tst-audit11mod1 tst-audit11mod2 tst-auditmod11 \
		tst-audit12mod1 tst-audit12mod2 tst-audit12mod3 tst-auditmod12 \
		tst-latepthreadmod $(tst-tls-many-dynamic-modules) \
		tst-nodelete-dlclose-dso tst-nodelete-dlclose-plugin \
		tst-main1mod tst-libc_dlvsym-dso tst-absolute-sym-lib \
		tst-absolute-zero-lib tst-big-note-lib

ifeq (yes,$(have-mtls-dialect-gnu2))
tests += tst-gnu2-tls1
modules-names += tst-gnu2-tls1mod
$(objpfx)tst-gnu2-tls1: $(objpfx)tst-gnu2-tls1mod.so
tst-gnu2-tls1mod.so-no-z-defs = yes
CFLAGS-tst-gnu2-tls1mod.c += -mtls-dialect=gnu2
endif
ifeq (yes,$(have-protected-data))
modules-names += tst-protected1moda tst-protected1modb
tests += tst-protected1a tst-protected1b
$(objpfx)tst-protected1a: $(addprefix $(objpfx),tst-protected1moda.so tst-protected1modb.so)
$(objpfx)tst-protected1b: $(addprefix $(objpfx),tst-protected1modb.so tst-protected1moda.so)
tst-protected1modb.so-no-z-defs = yes
# These tests fail with GCC versions prior to 5.1 and with some versions
# of binutils.  See https://sourceware.org/bugzilla/show_bug.cgi?id=17709
# and https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65248 for details.
# Perhaps in future we can make these XFAILs conditional on some detection
# of compiler/linker behavior/version.
test-xfail-tst-protected1a = yes
test-xfail-tst-protected1b = yes
endif
ifeq (yesyes,$(have-fpie)$(build-shared))
modules-names += tst-piemod1
tests += tst-pie1 tst-pie2
tests-pie += tst-pie1 tst-pie2
ifeq (yes,$(have-protected-data))
tests += vismain
tests-pie += vismain
CFLAGS-vismain.c += $(PIE-ccflag)
endif
endif
modules-execstack-yes = tst-execstack-mod
extra-test-objs += $(addsuffix .os,$(strip $(modules-names)))

# filtmod1.so has a special rule
modules-names-nobuild := filtmod1

tests += $(tests-static)

ifneq (no,$(multi-arch))
tests-ifuncstatic := ifuncmain1static ifuncmain1picstatic \
		ifuncmain2static ifuncmain2picstatic \
		ifuncmain4static ifuncmain4picstatic \
		ifuncmain5static ifuncmain5picstatic \
		ifuncmain7static ifuncmain7picstatic
tests-static += $(tests-ifuncstatic)
tests-internal += $(tests-ifuncstatic)
ifeq (yes,$(build-shared))
tests-internal += \
	 ifuncmain1 ifuncmain1pic ifuncmain1vis ifuncmain1vispic \
	 ifuncmain1staticpic \
	 ifuncmain2 ifuncmain2pic ifuncmain3 ifuncmain4 \
	 ifuncmain5 ifuncmain5pic ifuncmain5staticpic \
	 ifuncmain7 ifuncmain7pic
ifunc-test-modules = ifuncdep1 ifuncdep1pic ifuncdep2 ifuncdep2pic \
		     ifuncdep5 ifuncdep5pic
extra-test-objs += $(ifunc-test-modules:=.o)
test-internal-extras += $(ifunc-test-modules)
ifeq (yes,$(have-fpie))
ifunc-pie-tests = ifuncmain1pie ifuncmain1vispie ifuncmain1staticpie \
		  ifuncmain5pie ifuncmain6pie ifuncmain7pie
tests-internal += $(ifunc-pie-tests)
tests-pie += $(ifunc-pie-tests)
endif
modules-names += ifuncmod1 ifuncmod3 ifuncmod5 ifuncmod6
endif
endif

ifeq (yes,$(build-shared))
ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)tst-pathopt.out $(objpfx)tst-rtld-load-self.out
endif
tests-special += $(objpfx)check-textrel.out $(objpfx)check-execstack.out \
		 $(objpfx)check-localplt.out $(objpfx)check-initfini.out
endif

ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)order-cmp.out $(objpfx)tst-array1-cmp.out \
		 $(objpfx)tst-array1-static-cmp.out \
		 $(objpfx)tst-array2-cmp.out $(objpfx)tst-array3-cmp.out \
		 $(objpfx)tst-array4-cmp.out $(objpfx)tst-array5-cmp.out \
		 $(objpfx)tst-array5-static-cmp.out $(objpfx)order2-cmp.out \
		 $(objpfx)tst-initorder-cmp.out \
		 $(objpfx)tst-initorder2-cmp.out $(objpfx)tst-unused-dep.out \
		 $(objpfx)tst-unused-dep-cmp.out
endif

check-abi: $(objpfx)check-abi-ld.out
tests-special += $(objpfx)check-abi-ld.out
update-abi: update-abi-ld
update-all-abi: update-all-abi-ld

ifeq ($(have-glob-dat-reloc),yes)
tests += tst-prelink
# Don't compile tst-prelink.c with PIE for GLOB_DAT relocation.
CFLAGS-tst-prelink.c += -fno-pie
tst-prelink-no-pie = yes
ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)tst-prelink-cmp.out
endif
endif

# The test requires shared _and_ PIE because the executable
# unit test driver must be able to link with the shared object
# that is going to eventually go into an installed DSO.
ifeq (yesyes,$(have-fpie)$(build-shared))
tests-internal += tst-_dl_addr_inside_object
tests-pie += tst-_dl_addr_inside_object
$(objpfx)tst-_dl_addr_inside_object: $(objpfx)dl-addr-obj.os
CFLAGS-tst-_dl_addr_inside_object.c += $(PIE-ccflag)
endif

# We can only test static libcrypt use if libcrypt has been built,
# and either NSS crypto is not in use, or static NSS libraries are
# available.
ifeq ($(build-crypt),no)
CFLAGS-tst-linkall-static.c += -DUSE_CRYPT=0
else
ifeq ($(nss-crypt),no)
CFLAGS-tst-linkall-static.c += -DUSE_CRYPT=1
else
ifeq ($(static-nss-crypt),no)
CFLAGS-tst-linkall-static.c += -DUSE_CRYPT=0
else
CFLAGS-tst-linkall-static.c += -DUSE_CRYPT=1
endif
endif
endif

include ../Rules

ifeq (yes,$(build-shared))
# Make sure these things are built in the `make lib' pass so they can be used
# to run programs during the `make others' pass.
lib-noranlib: $(objpfx)$(rtld-installed-name) \
	      $(addprefix $(objpfx),$(extra-objs))
endif

# Command to link into a larger single relocatable object.
reloc-link = $(LINK.o) -nostdlib -nostartfiles -r

$(objpfx)sotruss-lib.so: $(shlib-lds)

$(objpfx)dl-allobjs.os: $(all-rtld-routines:%=$(objpfx)%.os)
	$(reloc-link) -o $@ $^

# Link together the dynamic linker into a single relocatable object.
# First we do a link against libc_pic.a just to get a link map,
# and discard the object produced by that link.  From the link map
# we can glean all the libc modules that need to go into the dynamic
# linker.  Then we do a recursive make that goes into all the subdirs
# those modules come from and builds special rtld-foo.os versions that
# are compiled with special flags, and puts these modules into rtld-libc.a
# for us.  Then we do the real link using rtld-libc.a instead of libc_pic.a.

# If the compiler can do SSP, build the mapfile with dummy __stack_chk_fail
# and __stack_chk_fail_local symbols defined, to prevent the real things
# being dragged into rtld even though rtld is never built with stack-
# protection.

ifeq ($(have-ssp),yes)
dummy-stack-chk-fail := -Wl,--defsym='__stack_chk_fail=0' \
			-Wl,--defsym='__stack_chk_fail_local=0'
else
dummy-stack-chk-fail :=
endif

$(objpfx)librtld.map: $(objpfx)dl-allobjs.os $(common-objpfx)libc_pic.a
	@-rm -f $@T
	$(reloc-link) -o $@.o $(dummy-stack-chk-fail) \
		'-Wl,-(' $^ -lgcc '-Wl,-)' -Wl,-Map,$@T
	rm -f $@.o
	mv -f $@T $@

$(objpfx)librtld.mk: $(objpfx)librtld.map Makefile
	LC_ALL=C \
	sed -n 's@^$(common-objpfx)\([^(]*\)(\([^)]*\.os\)) *.*$$@\1 \2@p' \
	    $< | \
	while read lib file; do \
	  case $$lib in \
	  libc_pic.a) \
	    LC_ALL=C fgrep -l /$$file \
		  $(common-objpfx)stamp.os $(common-objpfx)*/stamp.os | \
	    LC_ALL=C \
	    sed 's@^$(common-objpfx)\([^/]*\)/stamp\.os$$@rtld-\1'" +=$$file@"\
	    ;; \
	  */*.a) \
	    echo rtld-$${lib%%/*} += $$file ;; \
	  *) echo "Wasn't expecting $$lib($$file)" >&2; exit 1 ;; \
	  esac; \
	done > $@T
	echo rtld-subdirs = `LC_ALL=C sed 's/^rtld-\([^ ]*\).*$$/\1/' $@T \
			     | LC_ALL=C sort -u` >> $@T
	mv -f $@T $@

$(objpfx)rtld-libc.a: $(objpfx)librtld.mk FORCE
	$(MAKE) -f $< -f rtld-Rules

$(objpfx)librtld.os: $(objpfx)dl-allobjs.os $(objpfx)rtld-libc.a
	$(LINK.o) -nostdlib -nostartfiles -r -o $@ '-Wl,-(' $^ -lgcc '-Wl,-)' \
		  -Wl,-Map,$@.map

generated += librtld.map librtld.mk rtld-libc.a librtld.os.map

z-now-yes = -Wl,-z,now

$(objpfx)ld.so: $(objpfx)librtld.os $(ld-map)
# Link into a temporary file so that we don't touch $@ at all
# if the sanity check below fails.
	$(LINK.o) -nostdlib -nostartfiles -shared -o $@.new		\
		  $(LDFLAGS-rtld) -Wl,-z,defs $(z-now-$(bind-now))	\
		  $(filter-out $(map-file),$^) $(load-map-file)		\
		  -Wl,-soname=$(rtld-installed-name)			\
		  -Wl,-defsym=_begin=0
	$(call after-link,$@.new)
	$(READELF) -s $@.new \
	  | $(AWK) '($$7 ~ /^UND(|EF)$$/ && $$1 != "0:" && $$4 != "REGISTER") { print; p=1 } END { exit p != 0 }'
	mv -f $@.new $@

ifeq (yes,$(build-shared))
# interp.c exists just to get the runtime linker path into libc.so.
$(objpfx)interp.os: $(common-objpfx)runtime-linker.h
endif

ifneq (ld.so,$(rtld-installed-name))
# Make sure ld.so.1 exists in the build directory so we can link
# against it.
$(objpfx)$(rtld-installed-name): $(objpfx)ld.so
	$(make-link)
generated += $(rtld-installed-name)
endif

# Build a file mentioning all trustworthy directories to look for shared
# libraries when using LD_LIBRARY_PATH in a setuid program.  The user can
# add directories to the list by defining $(user-defined-trusted-dirs)
# before starting make.
$(objpfx)trusted-dirs.h: $(objpfx)trusted-dirs.st; @:
$(objpfx)trusted-dirs.st: Makefile $(..)Makeconfig
	$(make-target-directory)
	echo "$(subst :, ,$(default-rpath) $(user-defined-trusted-dirs))"    \
	| $(AWK) -f gen-trusted-dirs.awk > ${@:st=T};
	echo '#define DL_DST_LIB "$(notdir $(slibdir))"' >> ${@:st=T}
	$(move-if-change) ${@:st=T} ${@:st=h}
	touch $@
CPPFLAGS-dl-load.c += -I$(objpfx). -I$(csu-objpfx).

ifeq (yes,$(build-shared))
$(inst_slibdir)/$(rtld-version-installed-name): $(objpfx)ld.so $(+force)
	$(make-target-directory)
	$(do-install-program)

$(inst_rtlddir)/$(rtld-installed-name): \
  $(inst_slibdir)/$(rtld-version-installed-name) \
  $(inst_slibdir)/libc-$(version).so
	$(make-target-directory)
	$(make-shlib-link)

# Special target called by parent to install just the dynamic linker.
.PHONY: ldso_install
ldso_install: $(inst_rtlddir)/$(rtld-installed-name)
endif


ldd-rewrite = -e 's%@RTLD@%$(rtlddir)/$(rtld-installed-name)%g' \
	      -e 's%@VERSION@%$(version)%g' \
	      -e 's|@PKGVERSION@|$(PKGVERSION)|g' \
	      -e 's|@REPORT_BUGS_TO@|$(REPORT_BUGS_TO)|g' \
	      -e 's%@BASH@%$(BASH)%g' \
	      -e 's%@TEXTDOMAINDIR@%$(localedir)%g'

ifeq ($(ldd-rewrite-script),no)
define gen-ldd
LC_ALL=C sed $(ldd-rewrite) < $< > $@.new
endef
else
define gen-ldd
LC_ALL=C sed $(ldd-rewrite) < $< \
| LC_ALL=C sed -f $(patsubst $(..)/%,/%,$(..)$(ldd-rewrite-script)) > $@.new
endef
endif

$(objpfx)ldd: ldd.bash.in $(common-objpfx)soversions.mk \
	      $(common-objpfx)config.make
	$(gen-ldd)
	chmod 555 $@.new
	mv -f $@.new $@

$(objpfx)sprof: $(libdl)

$(objpfx)sln: $(sln-modules:%=$(objpfx)%.o)

$(objpfx)ldconfig: $(ldconfig-modules:%=$(objpfx)%.o)

SYSCONF-FLAGS := -D'SYSCONFDIR="$(sysconfdir)"'
CFLAGS-ldconfig.c += $(SYSCONF-FLAGS) -D'LIBDIR="$(libdir)"' \
		    -D'SLIBDIR="$(slibdir)"'
libof-ldconfig = ldconfig
CFLAGS-dl-cache.c += $(SYSCONF-FLAGS)
CFLAGS-cache.c += $(SYSCONF-FLAGS)
CFLAGS-rtld.c += $(SYSCONF-FLAGS)

cpp-srcs-left := $(all-rtld-routines:=.os)
lib := rtld
include $(patsubst %,$(..)libof-iterator.mk,$(cpp-srcs-left))

test-modules = $(addprefix $(objpfx),$(addsuffix .so,$(strip $(modules-names))))
generated += $(addsuffix .so,$(strip $(modules-names)))

$(objpfx)testobj1.so: $(libdl)
$(objpfx)testobj1_1.so: $(objpfx)testobj1.so $(libdl)
$(objpfx)testobj2.so: $(objpfx)testobj1.so $(libdl)
$(objpfx)testobj3.so: $(libdl)
$(objpfx)testobj4.so: $(libdl)
$(objpfx)testobj5.so: $(libdl)
$(objpfx)testobj6.so: $(objpfx)testobj1.so $(objpfx)testobj2.so $(libdl)
$(objpfx)failobj.so: $(objpfx)testobj6.so
$(objpfx)dep1.so: $(objpfx)dep2.so $(objpfx)dep4.so
$(objpfx)dep2.so: $(objpfx)dep3.so $(objpfx)dep4.so
$(objpfx)dep4.so: $(objpfx)dep3.so
$(objpfx)nodelmod3.so: $(objpfx)nodelmod4.so
$(objpfx)nextmod1.so: $(libdl)
$(objpfx)neededobj1.so: $(libdl)
$(objpfx)neededobj2.so: $(objpfx)neededobj1.so $(libdl)
$(objpfx)neededobj3.so: $(objpfx)neededobj1.so $(objpfx)neededobj2.so $(libdl)
$(objpfx)neededobj4.so: $(objpfx)neededobj1.so $(objpfx)neededobj2.so \
			$(objpfx)neededobj3.so $(libdl)
$(objpfx)neededobj6.so: $(objpfx)neededobj5.so
$(objpfx)unload2mod.so: $(objpfx)unload2dep.so
$(objpfx)ltglobmod2.so: $(libdl)
$(objpfx)firstobj.so: $(shared-thread-library)
$(objpfx)globalmod1.so: $(libdl)
$(objpfx)reldep4mod1.so: $(objpfx)reldep4mod3.so
$(objpfx)reldep4mod2.so: $(objpfx)reldep4mod4.so
$(objpfx)dblloadmod1.so: $(objpfx)dblloadmod3.so
$(objpfx)dblloadmod2.so: $(objpfx)dblloadmod3.so
$(objpfx)reldepmod5.so: $(objpfx)reldepmod2.so
$(objpfx)reldepmod6.so: $(objpfx)reldepmod2.so
$(objpfx)reldep6mod1.so: $(objpfx)reldep6mod0.so
$(objpfx)reldep6mod2.so: $(objpfx)reldep6mod1.so
$(objpfx)reldep6mod3.so: $(objpfx)reldep6mod2.so
$(objpfx)reldep6mod4.so: $(objpfx)reldep6mod1.so
$(objpfx)tst-tlsmod3.so: $(objpfx)tst-tlsmod2.so
$(objpfx)tst-tlsmod8.so: $(objpfx)tst-tlsmod7.so
$(objpfx)tst-tlsmod10.so: $(objpfx)tst-tlsmod9.so
$(objpfx)tst-tlsmod12.so: $(objpfx)tst-tlsmod11.so
$(objpfx)tst-tlsmod13a.so: $(objpfx)tst-tlsmod13.so
# For tst-tls9-static, make sure the modules it dlopens have libc.so in DT_NEEDED
$(objpfx)tst-tlsmod5.so: $(common-objpfx)libc.so
$(objpfx)tst-tlsmod6.so: $(common-objpfx)libc.so
$(objpfx)tst-tls19mod1.so: $(objpfx)tst-tls19mod2.so $(objpfx)tst-tls19mod3.so
$(objpfx)tst-tls19mod3.so: $(objpfx)ld.so
$(objpfx)reldep8mod3.so: $(objpfx)reldep8mod1.so $(objpfx)reldep8mod2.so
$(objpfx)nodel2mod3.so: $(objpfx)nodel2mod1.so $(objpfx)nodel2mod2.so
$(objpfx)reldep9mod2.so: $(objpfx)reldep9mod1.so
$(objpfx)reldep9mod3.so: $(objpfx)reldep9mod1.so $(objpfx)reldep9mod2.so
$(objpfx)unload3mod1.so: $(objpfx)unload3mod3.so
$(objpfx)unload3mod2.so: $(objpfx)unload3mod3.so
$(objpfx)unload3mod3.so: $(objpfx)unload3mod4.so
$(objpfx)unload4mod1.so: $(objpfx)unload4mod2.so $(objpfx)unload4mod3.so
$(objpfx)unload4mod2.so: $(objpfx)unload4mod4.so $(objpfx)unload4mod3.so
$(objpfx)unload6mod1.so: $(libdl)
$(objpfx)unload6mod2.so: $(libdl)
$(objpfx)unload6mod3.so: $(libdl)
$(objpfx)unload7mod1.so: $(libdl)
$(objpfx)unload7mod2.so: $(objpfx)unload7mod1.so
$(objpfx)unload8mod1.so: $(objpfx)unload8mod2.so
$(objpfx)unload8mod2.so: $(objpfx)unload8mod3.so
$(objpfx)unload8mod3.so: $(libdl)
$(objpfx)tst-initordera2.so: $(objpfx)tst-initordera1.so
$(objpfx)tst-initorderb2.so: $(objpfx)tst-initorderb1.so $(objpfx)tst-initordera2.so
$(objpfx)tst-initordera3.so: $(objpfx)tst-initorderb2.so $(objpfx)tst-initorderb1.so
$(objpfx)tst-initordera4.so: $(objpfx)tst-initordera3.so
$(objpfx)tst-initorder: $(objpfx)tst-initordera4.so $(objpfx)tst-initordera1.so $(objpfx)tst-initorderb2.so
$(objpfx)tst-null-argv: $(objpfx)tst-null-argv-lib.so
$(objpfx)tst-tlsalign: $(objpfx)tst-tlsalign-lib.so
$(objpfx)tst-nodelete-opened.out: $(objpfx)tst-nodelete-opened-lib.so
$(objpfx)tst-nodelete-opened: $(libdl)
$(objpfx)tst-noload: $(libdl)

$(objpfx)tst-tlsalign-extern: $(objpfx)tst-tlsalign-vars.o
$(objpfx)tst-tlsalign-extern-static: $(objpfx)tst-tlsalign-vars.o

tst-null-argv-ENV = LD_DEBUG=all LD_DEBUG_OUTPUT=$(objpfx)tst-null-argv.debug.out
LDFLAGS-nodel2mod3.so = $(no-as-needed)
LDFLAGS-reldepmod5.so = $(no-as-needed)
LDFLAGS-reldep6mod1.so = $(no-as-needed)
LDFLAGS-reldep6mod4.so = $(no-as-needed)
LDFLAGS-reldep8mod3.so = $(no-as-needed)
LDFLAGS-unload4mod1.so = $(no-as-needed)
LDFLAGS-unload4mod2.so = $(no-as-needed)
LDFLAGS-tst-initorder = $(no-as-needed)
LDFLAGS-tst-initordera2.so = $(no-as-needed)
LDFLAGS-tst-initordera3.so = $(no-as-needed)
LDFLAGS-tst-initordera4.so = $(no-as-needed)
LDFLAGS-tst-initorderb2.so = $(no-as-needed)
LDFLAGS-tst-tlsmod5.so = -nostdlib $(no-as-needed)
LDFLAGS-tst-tlsmod6.so = -nostdlib $(no-as-needed)

testobj1.so-no-z-defs = yes
testobj3.so-no-z-defs = yes
testobj4.so-no-z-defs = yes
testobj5.so-no-z-defs = yes
testobj6.so-no-z-defs = yes
failobj.so-no-z-defs = yes
constload2.so-no-z-defs = yes
constload3.so-no-z-defs = yes
nodelmod1.so-no-z-defs = yes
nodelmod2.so-no-z-defs = yes
nodelmod4.so-no-z-defs = yes
nodel2mod2.so-no-z-defs = yes
reldepmod2.so-no-z-defs = yes
reldepmod3.so-no-z-defs = yes
reldepmod4.so-no-z-defs = yes
reldep4mod4.so-no-z-defs = yes
reldep4mod2.so-no-z-defs = yes
ltglobmod2.so-no-z-defs = yes
dblloadmod3.so-no-z-defs = yes
tst-tlsmod1.so-no-z-defs = yes
tst-tlsmod2.so-no-z-defs = yes
tst-tlsmod3.so-no-z-defs = yes
tst-tlsmod4.so-no-z-defs = yes
tst-tlsmod7.so-no-z-defs = yes
tst-tlsmod8.so-no-z-defs = yes
tst-tlsmod9.so-no-z-defs = yes
tst-tlsmod10.so-no-z-defs = yes
tst-tlsmod12.so-no-z-defs = yes
tst-tlsmod14a.so-no-z-defs = yes
tst-tlsmod14b.so-no-z-defs = yes
tst-tlsmod15a.so-no-z-defs = yes
tst-tlsmod16b.so-no-z-defs = yes
circlemod2.so-no-z-defs = yes
circlemod3.so-no-z-defs = yes
circlemod3a.so-no-z-defs = yes
reldep8mod2.so-no-z-defs = yes
reldep9mod1.so-no-z-defs = yes
unload3mod4.so-no-z-defs = yes
unload4mod1.so-no-z-defs = yes
ifuncmod1.so-no-z-defs = yes
ifuncmod5.so-no-z-defs = yes
ifuncmod6.so-no-z-defs = yes
tst-auditmod9a.so-no-z-defs = yes
tst-auditmod9b.so-no-z-defs = yes
tst-nodelete-uniquemod.so-no-z-defs = yes
tst-nodelete-rtldmod.so-no-z-defs = yes
tst-nodelete-zmod.so-no-z-defs = yes
tst-nodelete2mod.so-no-z-defs = yes

ifeq ($(build-shared),yes)
# Build all the modules even when not actually running test programs.
tests: $(test-modules)
endif

$(objpfx)loadtest: $(libdl)
LDFLAGS-loadtest = -rdynamic

$(objpfx)loadtest.out: $(test-modules)

$(objpfx)neededtest: $(libdl)
$(objpfx)neededtest.out: $(objpfx)neededobj1.so $(objpfx)neededobj2.so \
			 $(objpfx)neededobj3.so

$(objpfx)neededtest2: $(libdl)
$(objpfx)neededtest2.out: $(objpfx)neededobj1.so $(objpfx)neededobj2.so \
			  $(objpfx)neededobj3.so

$(objpfx)neededtest3: $(libdl)
$(objpfx)neededtest3.out: $(objpfx)neededobj1.so $(objpfx)neededobj2.so \
			  $(objpfx)neededobj3.so $(objpfx)neededobj4.so

$(objpfx)neededtest4: $(libdl) $(objpfx)neededobj1.so
$(objpfx)neededtest4.out: $(objpfx)neededobj5.so $(objpfx)neededobj6.so

$(objpfx)restest1: $(objpfx)testobj1.so $(objpfx)testobj1_1.so $(libdl)
LDFLAGS-restest1 = -rdynamic

$(objpfx)restest2: $(libdl)
LDFLAGS-restest2 = -rdynamic

$(objpfx)restest1.out: $(test-modules)

preloadtest-preloads = testobj1 testobj2 testobj3 testobj4 testobj5
$(objpfx)preloadtest: $(objpfx)testobj6.so
LDFLAGS-preloadtest = -rdynamic
$(objpfx)preloadtest.out: $(preloadtest-preloads:%=$(objpfx)%.so)
preloadtest-ENV = \
  LD_PRELOAD=$(subst $(empty) ,:,$(strip $(preloadtest-preloads:=.so)))

$(objpfx)loadfail: $(libdl)
LDFLAGS-loadfail = -rdynamic

$(objpfx)loadfail.out: $(objpfx)failobj.so

$(objpfx)multiload: $(libdl)
LDFLAGS-multiload = -rdynamic
CFLAGS-multiload.c += -DOBJDIR=\"$(elf-objpfx)\"

$(objpfx)multiload.out: $(objpfx)testobj1.so

$(objpfx)origtest: $(libdl)
LDFLAGS-origtest = -rdynamic
$(objpfx)origtest.out: $(objpfx)testobj1.so

ifeq ($(have-thread-library),yes)
$(objpfx)resolvfail: $(libdl) $(shared-thread-library)
else
$(objpfx)resolvfail: $(libdl)
endif

$(objpfx)constload1: $(libdl)
$(objpfx)constload1.out: $(objpfx)constload2.so $(objpfx)constload3.so

$(objpfx)circleload1: $(libdl)
$(objpfx)circleload1.out: $(objpfx)circlemod1.so \
			  $(objpfx)circlemod1a.so

$(objpfx)circlemod1.so: $(objpfx)circlemod2.so
$(objpfx)circlemod2.so: $(objpfx)circlemod3.so
$(objpfx)circlemod1a.so: $(objpfx)circlemod2a.so
$(objpfx)circlemod2a.so: $(objpfx)circlemod3a.so

$(objpfx)order: $(addprefix $(objpfx),dep4.so dep3.so dep2.so dep1.so)

$(objpfx)order-cmp.out: $(objpfx)order.out
	(echo "0123456789" | cmp $< -) > $@; \
	$(evaluate-test)

$(objpfx)vismain: $(addprefix $(objpfx),vismod1.so vismod2.so)
$(objpfx)vismain.out: $(addprefix $(objpfx),vismod3.so)
vismain-ENV = LD_PRELOAD=$(addprefix $(objpfx),vismod3.so)

$(objpfx)noload: $(objpfx)testobj1.so $(libdl)
LDFLAGS-noload = -rdynamic $(no-as-needed)
$(objpfx)noload.out: $(objpfx)testobj5.so

$(objpfx)noload-mem.out: $(objpfx)noload.out
	$(common-objpfx)malloc/mtrace $(objpfx)noload.mtrace > $@; \
	$(evaluate-test)
noload-ENV = MALLOC_TRACE=$(objpfx)noload.mtrace

LDFLAGS-nodelete = -rdynamic
LDFLAGS-nodelmod1.so = -Wl,--enable-new-dtags,-z,nodelete
LDFLAGS-nodelmod4.so = -Wl,--enable-new-dtags,-z,nodelete
$(objpfx)nodelete: $(libdl)
$(objpfx)nodelete.out: $(objpfx)nodelmod1.so $(objpfx)nodelmod2.so \
		       $(objpfx)nodelmod3.so

LDFLAGS-nodlopenmod.so = -Wl,--enable-new-dtags,-z,nodlopen
$(objpfx)nodlopen: $(libdl)
$(objpfx)nodlopen.out: $(objpfx)nodlopenmod.so

$(objpfx)nodlopenmod2.so: $(objpfx)nodlopenmod.so
$(objpfx)nodlopen2: $(libdl)
$(objpfx)nodlopen2.out: $(objpfx)nodlopenmod2.so

$(objpfx)filtmod1.so: $(objpfx)filtmod1.os $(objpfx)filtmod2.so
	$(LINK.o) -shared -o $@ -B$(csu-objpfx) $(LDFLAGS.so) \
		  -L$(subst :, -L,$(rpath-link)) \
		  -Wl,-rpath-link=$(rpath-link) \
		  $< -Wl,-F,$(objpfx)filtmod2.so
$(objpfx)filter: $(objpfx)filtmod1.so

# This does not link against libc.
CFLAGS-filtmod1.c += $(no-stack-protector)

$(objpfx)unload: $(libdl)
$(objpfx)unload.out: $(objpfx)unloadmod.so

$(objpfx)reldep: $(libdl)
$(objpfx)reldep.out: $(objpfx)reldepmod1.so $(objpfx)reldepmod2.so

$(objpfx)reldep2: $(libdl)
$(objpfx)reldep2.out: $(objpfx)reldepmod1.so $(objpfx)reldepmod3.so

$(objpfx)reldep3: $(libdl)
$(objpfx)reldep3.out: $(objpfx)reldepmod1.so $(objpfx)reldepmod4.so

$(objpfx)reldep4: $(libdl)
$(objpfx)reldep4.out: $(objpfx)reldep4mod1.so $(objpfx)reldep4mod2.so

$(objpfx)next: $(objpfx)nextmod1.so $(objpfx)nextmod2.so $(libdl)
LDFLAGS-next = $(no-as-needed)

$(objpfx)unload2: $(libdl)
$(objpfx)unload2.out: $(objpfx)unload2mod.so $(objpfx)unload2dep.so

$(objpfx)lateglobal: $(libdl)
$(objpfx)lateglobal.out: $(objpfx)ltglobmod1.so $(objpfx)ltglobmod2.so

$(objpfx)tst-pathopt: $(libdl)
$(objpfx)tst-pathopt.out: tst-pathopt.sh $(objpfx)tst-pathopt \
			  $(objpfx)pathoptobj.so
	$(SHELL) $< $(common-objpfx) '$(test-wrapper-env)' \
		 '$(run-program-env)' > $@; \
	$(evaluate-test)

$(objpfx)tst-rtld-load-self.out: tst-rtld-load-self.sh $(objpfx)ld.so
	$(SHELL) $^ '$(test-wrapper)' '$(test-wrapper-env)' > $@; \
	$(evaluate-test)

$(objpfx)initfirst: $(libdl)
$(objpfx)initfirst.out: $(objpfx)firstobj.so

$(objpfx)global: $(objpfx)globalmod1.so
$(objpfx)global.out: $(objpfx)reldepmod1.so

$(objpfx)dblload: $(libdl)
$(objpfx)dblload.out: $(objpfx)dblloadmod1.so $(objpfx)dblloadmod2.so

$(objpfx)dblunload: $(libdl)
$(objpfx)dblunload.out: $(objpfx)dblloadmod1.so $(objpfx)dblloadmod2.so

$(objpfx)reldep5: $(libdl)
$(objpfx)reldep5.out: $(objpfx)reldepmod5.so $(objpfx)reldepmod6.so

$(objpfx)reldep6: $(libdl)
$(objpfx)reldep6.out: $(objpfx)reldep6mod3.so $(objpfx)reldep6mod4.so

$(objpfx)reldep7: $(libdl)
$(objpfx)reldep7.out: $(objpfx)reldep7mod1.so $(objpfx)reldep7mod2.so

$(objpfx)reldep8: $(libdl)
$(objpfx)reldep8.out: $(objpfx)reldep8mod3.so

LDFLAGS-nodel2mod2.so = -Wl,--enable-new-dtags,-z,nodelete
$(objpfx)nodelete2: $(libdl)
$(objpfx)nodelete2.out: $(objpfx)nodel2mod3.so

$(objpfx)reldep9: $(libdl)
$(objpfx)reldep9.out: $(objpfx)reldep9mod3.so

$(objpfx)tst-tls3: $(objpfx)tst-tlsmod1.so

$(objpfx)tst-tls4: $(libdl)
$(objpfx)tst-tls4.out: $(objpfx)tst-tlsmod2.so

$(objpfx)tst-tls5: $(libdl)
$(objpfx)tst-tls5.out: $(objpfx)tst-tlsmod2.so

$(objpfx)tst-tls6: $(libdl)
$(objpfx)tst-tls6.out: $(objpfx)tst-tlsmod2.so

$(objpfx)tst-tls7: $(libdl)
$(objpfx)tst-tls7.out: $(objpfx)tst-tlsmod3.so

$(objpfx)tst-tls8: $(libdl)
$(objpfx)tst-tls8.out: $(objpfx)tst-tlsmod3.so $(objpfx)tst-tlsmod4.so

$(objpfx)tst-tls9: $(libdl)
$(objpfx)tst-tls9.out: $(objpfx)tst-tlsmod5.so $(objpfx)tst-tlsmod6.so

$(objpfx)tst-tls10: $(objpfx)tst-tlsmod8.so $(objpfx)tst-tlsmod7.so

$(objpfx)tst-tls11: $(objpfx)tst-tlsmod10.so $(objpfx)tst-tlsmod9.so

$(objpfx)tst-tls12: $(objpfx)tst-tlsmod12.so $(objpfx)tst-tlsmod11.so

$(objpfx)tst-tls13: $(libdl)
$(objpfx)tst-tls13.out: $(objpfx)tst-tlsmod13a.so

$(objpfx)tst-tls14: $(objpfx)tst-tlsmod14a.so $(libdl)
$(objpfx)tst-tls14.out: $(objpfx)tst-tlsmod14b.so

$(objpfx)tst-tls15: $(libdl)
$(objpfx)tst-tls15.out: $(objpfx)tst-tlsmod15a.so $(objpfx)tst-tlsmod15b.so

$(objpfx)tst-tls-dlinfo: $(libdl)
$(objpfx)tst-tls-dlinfo.out: $(objpfx)tst-tlsmod2.so



$(objpfx)tst-tls16: $(libdl)
$(objpfx)tst-tls16.out: $(objpfx)tst-tlsmod16a.so $(objpfx)tst-tlsmod16b.so

$(objpfx)tst-tls17: $(libdl)
$(objpfx)tst-tls17.out: $(objpfx)tst-tlsmod17b.so
$(patsubst %,$(objpfx)%.os,$(tlsmod17a-modules)): $(objpfx)tst-tlsmod17a%.os: tst-tlsmod17a.c
	$(compile-command.c) -DN=$*
$(patsubst %,$(objpfx)%.so,$(tlsmod17a-modules)): $(objpfx)tst-tlsmod17a%.so: $(objpfx)ld.so
$(objpfx)tst-tlsmod17b.so: $(patsubst %,$(objpfx)%.so,$(tlsmod17a-modules))

$(objpfx)tst-tls18: $(libdl)
$(objpfx)tst-tls18.out: $(patsubst %,$(objpfx)%.so,$(tlsmod18a-modules))
$(patsubst %,$(objpfx)%.os,$(tlsmod18a-modules)): $(objpfx)tst-tlsmod18a%.os : tst-tlsmod18a.c
	$(compile-command.c) -DN=$*
$(patsubst %,$(objpfx)%.so,$(tlsmod18a-modules)): $(objpfx)tst-tlsmod18a%.so: $(objpfx)ld.so

$(objpfx)tst-tls19: $(libdl)
$(objpfx)tst-tls19.out: $(objpfx)tst-tls19mod1.so

CFLAGS-tst-align.c += $(stack-align-test-flags)
CFLAGS-tst-align2.c += $(stack-align-test-flags)
CFLAGS-tst-alignmod.c += $(stack-align-test-flags)
CFLAGS-tst-alignmod2.c += $(stack-align-test-flags)
$(objpfx)tst-align: $(libdl)
$(objpfx)tst-align.out: $(objpfx)tst-alignmod.so
$(objpfx)tst-align2: $(objpfx)tst-alignmod2.so

$(objpfx)unload3: $(libdl)
$(objpfx)unload3.out: $(objpfx)unload3mod1.so $(objpfx)unload3mod2.so \
		      $(objpfx)unload3mod3.so $(objpfx)unload3mod4.so

$(objpfx)unload4: $(libdl)
$(objpfx)unload4.out: $(objpfx)unload4mod1.so $(objpfx)unload4mod3.so

$(objpfx)unload5: $(libdl)
$(objpfx)unload5.out: $(objpfx)unload3mod1.so $(objpfx)unload3mod2.so \
		      $(objpfx)unload3mod3.so $(objpfx)unload3mod4.so

$(objpfx)unload6: $(libdl)
$(objpfx)unload6.out: $(objpfx)unload6mod1.so $(objpfx)unload6mod2.so \
		      $(objpfx)unload6mod3.so

$(objpfx)unload7: $(libdl)
$(objpfx)unload7.out: $(objpfx)unload7mod1.so $(objpfx)unload7mod2.so
unload7-ENV = MALLOC_PERTURB_=85

$(objpfx)unload8: $(libdl)
$(objpfx)unload8.out: $(objpfx)unload8mod1.so $(objpfx)unload8mod1x.so

ifdef libdl
$(objpfx)tst-tls9-static: $(common-objpfx)dlfcn/libdl.a
$(objpfx)tst-tls9-static.out: $(objpfx)tst-tlsmod5.so $(objpfx)tst-tlsmod6.so
endif

ifeq ($(have-z-execstack),yes)
$(objpfx)tst-execstack: $(libdl)
$(objpfx)tst-execstack.out: $(objpfx)tst-execstack-mod.so
CPPFLAGS-tst-execstack.c += -DUSE_PTHREADS=0
LDFLAGS-tst-execstack = -Wl,-z,noexecstack
LDFLAGS-tst-execstack-mod = -Wl,-z,execstack

$(objpfx)tst-execstack-needed: $(objpfx)tst-execstack-mod.so
LDFLAGS-tst-execstack-needed = -Wl,-z,noexecstack

LDFLAGS-tst-execstack-prog = -Wl,-z,execstack
CFLAGS-tst-execstack-prog.c += -Wno-trampolines
CFLAGS-tst-execstack-mod.c += -Wno-trampolines
endif

LDFLAGS-tst-array2 = $(no-as-needed)
LDFLAGS-tst-array5 = $(no-as-needed)

$(objpfx)tst-array1-cmp.out: tst-array1.exp $(objpfx)tst-array1.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-array1-static-cmp.out: tst-array1.exp \
				    $(objpfx)tst-array1-static.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-array2: $(objpfx)tst-array2dep.so
$(objpfx)tst-array2-cmp.out: tst-array2.exp $(objpfx)tst-array2.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-array3-cmp.out: tst-array1.exp $(objpfx)tst-array3.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-array4: $(libdl)
$(objpfx)tst-array4.out: $(objpfx)tst-array2dep.so
$(objpfx)tst-array4-cmp.out: tst-array4.exp $(objpfx)tst-array4.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-array5: $(objpfx)tst-array5dep.so
$(objpfx)tst-array5-cmp.out: tst-array5.exp $(objpfx)tst-array5.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-array5-static-cmp.out: tst-array5-static.exp \
				$(objpfx)tst-array5-static.out
	cmp $^ > $@; \
	$(evaluate-test)

CFLAGS-tst-pie1.c += $(pie-ccflag)
CFLAGS-tst-pie2.c += $(pie-ccflag)

$(objpfx)tst-piemod1.so: $(libsupport)
$(objpfx)tst-pie1: $(objpfx)tst-piemod1.so

ifeq (yes,$(build-shared))
# NB: Please keep cet-built-dso in sysdeps/x86/Makefile in sync with
# all-built-dso here.
all-built-dso := $(common-objpfx)elf/ld.so $(common-objpfx)libc.so \
		 $(filter-out $(common-objpfx)linkobj/libc.so, \
			      $(sort $(wildcard $(addprefix $(common-objpfx), \
							    */lib*.so \
							    iconvdata/*.so))))

$(all-built-dso:=.dyn): %.dyn: %
	@rm -f $@T
	LC_ALL=C $(READELF) -W -d $< > $@T
	test -s $@T
	mv -f $@T $@
common-generated += $(all-built-dso:$(common-objpfx)%=%.dyn)

$(objpfx)check-textrel.out: $(..)scripts/check-textrel.awk \
			    $(all-built-dso:=.dyn)
	LC_ALL=C $(AWK) -f $^ > $@; \
	$(evaluate-test)
generated += check-textrel.out

$(objpfx)execstack-default: $(first-word $(wildcard $(sysdirs:%=%/stackinfo.h)))
	$(make-target-directory)
	{ echo '#include <elf.h>'; \
	  echo '#include <stackinfo.h>'; \
	  echo '#if (DEFAULT_STACK_PERMS & PF_X) == 0'; \
	  echo '@@@execstack-no@@@'; \
	  echo '#else'; \
	  echo '@@@execstack-yes@@@'; \
	  echo '#endif'; } | \
	$(CC) $(CFLAGS) $(CPPFLAGS) -E -x c-header - | \
	sed -n -e 's/^@@@\(.*\)@@@/\1/p' > $@T
	mv -f $@T $@
generated += execstack-default

$(all-built-dso:=.phdr): %.phdr: %
	@rm -f $@T
	LC_ALL=C $(READELF) -W -l $< > $@T
	test -s $@T
	mv -f $@T $@
common-generated += $(all-built-dso:$(common-objpfx)%=%.phdr)

$(objpfx)check-execstack.out: $(..)scripts/check-execstack.awk \
			      $(objpfx)execstack-default \
			      $(all-built-dso:=.phdr)
	LC_ALL=C $(AWK) -v "xfail=$(check-execstack-xfail)" -f $^ > $@; \
	$(evaluate-test)
generated += check-execstack.out

$(objpfx)tst-dlmodcount: $(libdl)
$(objpfx)tst-dlmodcount.out: $(test-modules)

$(all-built-dso:=.jmprel): %.jmprel: % Makefile
	@rm -f $@T
	LC_ALL=C $(READELF) -W -S -d -r $< > $@T
	test -s $@T
	mv -f $@T $@
common-generated += $(all-built-dso:$(common-objpfx)%=%.jmprel)

localplt-built-dso := $(addprefix $(common-objpfx),\
				  libc.so \
				  elf/ld.so \
				  math/libm.so \
				  rt/librt.so \
				  dlfcn/libdl.so \
				  resolv/libresolv.so \
		       )
ifeq ($(build-mathvec),yes)
localplt-built-dso += $(addprefix $(common-objpfx), mathvec/libmvec.so)
endif
ifeq ($(have-thread-library),yes)
localplt-built-dso += $(filter-out %_nonshared.a, $(shared-thread-library))
endif
ifeq ($(build-crypt),yes)
localplt-built-dso += $(addprefix $(common-objpfx), crypt/libcrypt.so)
endif

vpath localplt.data $(+sysdep_dirs)

$(objpfx)check-localplt.out: $(..)scripts/check-localplt.awk \
			     $(..)scripts/localplt.awk \
			     $(localplt-built-dso:=.jmprel) \
			     localplt.data
	LC_ALL=C $(AWK) -f $(filter-out $< %localplt.data,$^) | \
	  LC_ALL=C $(AWK) -f $< $(filter %localplt.data,$^) - \
	  > $@; \
	$(evaluate-test)
endif

$(all-built-dso:=.dynsym): %.dynsym: %
	@rm -f $@T
	LC_ALL=C $(READELF) -W --dyn-syms $< > $@T
	test -s $@T
	mv -f $@T $@
common-generated += $(all-built-dso:$(common-objpfx)%=%.dynsym)

$(objpfx)check-initfini.out: $(..)scripts/check-initfini.awk \
			    $(all-built-dso:=.dynsym)
	LC_ALL=C $(AWK) -f $^ > $@; \
	$(evaluate-test)
generated += check-initfini.out

$(objpfx)tst-dlopenrpathmod.so: $(libdl)
$(objpfx)tst-dlopenrpath: $(objpfx)tst-dlopenrpathmod.so $(libdl)
CFLAGS-tst-dlopenrpath.c += -DPFX=\"$(objpfx)\"
LDFLAGS-tst-dlopenrpathmod.so += -Wl,-rpath,\$$ORIGIN/test-subdir
$(objpfx)tst-dlopenrpath.out: $(objpfx)firstobj.so

$(objpfx)tst-deep1mod2.so: $(objpfx)tst-deep1mod3.so
$(objpfx)tst-deep1: $(libdl) $(objpfx)tst-deep1mod1.so
$(objpfx)tst-deep1.out: $(objpfx)tst-deep1mod2.so
LDFLAGS-tst-deep1 += -rdynamic
tst-deep1mod3.so-no-z-defs = yes

$(objpfx)tst-dlmopen1mod.so: $(libdl)
$(objpfx)tst-dlmopen1: $(libdl)
$(objpfx)tst-dlmopen1.out: $(objpfx)tst-dlmopen1mod.so

$(objpfx)tst-dlmopen2: $(libdl)
$(objpfx)tst-dlmopen2.out: $(objpfx)tst-dlmopen1mod.so

$(objpfx)tst-dlmopen3: $(libdl)
$(objpfx)tst-dlmopen3.out: $(objpfx)tst-dlmopen1mod.so

$(objpfx)tst-audit1.out: $(objpfx)tst-auditmod1.so
tst-audit1-ENV = LD_AUDIT=$(objpfx)tst-auditmod1.so

$(objpfx)tst-audit2: $(libdl)
$(objpfx)tst-audit2.out: $(objpfx)tst-auditmod1.so $(objpfx)tst-auditmod9b.so
# Prevent GCC-5 from translating a malloc/memset pair into calloc
CFLAGS-tst-audit2.c += -fno-builtin
tst-audit2-ENV = LD_AUDIT=$(objpfx)tst-auditmod1.so

$(objpfx)tst-audit9: $(libdl)
$(objpfx)tst-audit9.out: $(objpfx)tst-auditmod9a.so $(objpfx)tst-auditmod9b.so
tst-audit9-ENV = LD_AUDIT=$(objpfx)tst-auditmod9a.so

$(objpfx)tst-audit8: $(libm)
$(objpfx)tst-audit8.out: $(objpfx)tst-auditmod1.so
tst-audit8-ENV = LD_AUDIT=$(objpfx)tst-auditmod1.so

$(objpfx)tst-global1: $(libdl)
$(objpfx)tst-global1.out: $(objpfx)testobj6.so $(objpfx)testobj2.so

$(objpfx)order2: $(libdl)
$(objpfx)order2.out: $(objpfx)order2mod1.so $(objpfx)order2mod2.so
$(objpfx)order2-cmp.out: $(objpfx)order2.out
	(echo "12345" | cmp $< -) > $@; \
	$(evaluate-test)
$(objpfx)order2mod1.so: $(objpfx)order2mod4.so
$(objpfx)order2mod4.so: $(objpfx)order2mod3.so
$(objpfx)order2mod2.so: $(objpfx)order2mod3.so
order2mod2.so-no-z-defs = yes
LDFLAGS-order2mod1.so = $(no-as-needed)
LDFLAGS-order2mod2.so = $(no-as-needed)

tst-stackguard1-ARGS = --command "$(host-test-program-cmd) --child"
tst-stackguard1-static-ARGS = --command "$(objpfx)tst-stackguard1-static --child"

tst-ptrguard1-ARGS = --command "$(host-test-program-cmd) --child"
# When built statically, the pointer guard interface uses
# __pointer_chk_guard_local.
CFLAGS-tst-ptrguard1-static.c += -DPTRGUARD_LOCAL
tst-ptrguard1-static-ARGS = --command "$(objpfx)tst-ptrguard1-static --child"

$(objpfx)tst-leaks1: $(libdl)
$(objpfx)tst-leaks1-mem.out: $(objpfx)tst-leaks1.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-leaks1.mtrace > $@; \
	$(evaluate-test)

$(objpfx)tst-leaks1-static: $(common-objpfx)dlfcn/libdl.a
$(objpfx)tst-leaks1-static-mem.out: $(objpfx)tst-leaks1-static.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-leaks1-static.mtrace > $@; \
	$(evaluate-test)

tst-leaks1-ENV = MALLOC_TRACE=$(objpfx)tst-leaks1.mtrace
tst-leaks1-static-ENV = MALLOC_TRACE=$(objpfx)tst-leaks1-static.mtrace

$(objpfx)tst-addr1: $(libdl)

$(objpfx)tst-thrlock: $(libdl) $(shared-thread-library)
$(objpfx)tst-dlopen-aout: $(libdl) $(shared-thread-library)

CFLAGS-ifuncmain1pic.c += $(pic-ccflag)
CFLAGS-ifuncmain1picstatic.c += $(pic-ccflag)
CFLAGS-ifuncmain1staticpic.c += $(pic-ccflag)
CFLAGS-ifuncdep1pic.c += $(pic-ccflag)
CFLAGS-ifuncmain1vispic.c += $(pic-ccflag)
CFLAGS-ifuncmain2pic.c += $(pic-ccflag)
CFLAGS-ifuncmain2picstatic.c += $(pic-ccflag)
CFLAGS-ifuncdep2pic.c += $(pic-ccflag)
CFLAGS-ifuncmain4picstatic.c += $(pic-ccflag)
CFLAGS-ifuncmain5pic.c += $(pic-ccflag)
CFLAGS-ifuncmain5picstatic.c += $(pic-ccflag)
CFLAGS-ifuncmain5staticpic.c += $(pic-ccflag)
CFLAGS-ifuncdep5pic.c += $(pic-ccflag)
CFLAGS-ifuncmain7pic.c += $(pic-ccflag)
CFLAGS-ifuncmain7picstatic.c += $(pic-ccflag)

LDFLAGS-ifuncmain3 = -Wl,-export-dynamic

CFLAGS-ifuncmain1pie.c += $(pie-ccflag)
CFLAGS-ifuncmain1vispie.c += $(pie-ccflag)
CFLAGS-ifuncmain1staticpie.c += $(pie-ccflag)
CFLAGS-ifuncmain5pie.c += $(pie-ccflag)
CFLAGS-ifuncmain6pie.c += $(pie-ccflag)
CFLAGS-ifuncmain7pie.c += $(pie-ccflag)

$(objpfx)ifuncmain1pie: $(objpfx)ifuncmod1.so
$(objpfx)ifuncmain1staticpie: $(objpfx)ifuncdep1pic.o
$(objpfx)ifuncmain1vispie: $(objpfx)ifuncmod1.so
$(objpfx)ifuncmain5pie: $(objpfx)ifuncmod5.so
$(objpfx)ifuncmain6pie: $(objpfx)ifuncmod6.so

$(objpfx)ifuncmain1: $(addprefix $(objpfx),ifuncmod1.so)
$(objpfx)ifuncmain1pic: $(addprefix $(objpfx),ifuncmod1.so)
$(objpfx)ifuncmain1staticpic: $(addprefix $(objpfx),ifuncdep1pic.o)
$(objpfx)ifuncmain1static: $(addprefix $(objpfx),ifuncdep1.o)
$(objpfx)ifuncmain1picstatic: $(addprefix $(objpfx),ifuncdep1pic.o)
$(objpfx)ifuncmain1vis: $(addprefix $(objpfx),ifuncmod1.so)
$(objpfx)ifuncmain1vispic: $(addprefix $(objpfx),ifuncmod1.so)
$(objpfx)ifuncmain2: $(addprefix $(objpfx),ifuncdep2.o)
$(objpfx)ifuncmain2pic: $(addprefix $(objpfx),ifuncdep2pic.o)
$(objpfx)ifuncmain2static: $(addprefix $(objpfx),ifuncdep2.o)
$(objpfx)ifuncmain2picstatic: $(addprefix $(objpfx),ifuncdep2pic.o)

$(objpfx)ifuncmain3: $(libdl)
$(objpfx)ifuncmain3.out: $(objpfx)ifuncmod3.so

$(objpfx)ifuncmain5: $(addprefix $(objpfx),ifuncmod5.so)
$(objpfx)ifuncmain5pic: $(addprefix $(objpfx),ifuncmod5.so)
$(objpfx)ifuncmain5static: $(addprefix $(objpfx),ifuncdep5.o)
$(objpfx)ifuncmain5staticpic: $(addprefix $(objpfx),ifuncdep5pic.o)
$(objpfx)ifuncmain5picstatic: $(addprefix $(objpfx),ifuncdep5pic.o)

$(objpfx)tst-unique1: $(libdl)
$(objpfx)tst-unique1.out: $(objpfx)tst-unique1mod1.so \
			  $(objpfx)tst-unique1mod2.so

$(objpfx)tst-unique2: $(libdl) $(objpfx)tst-unique2mod1.so
$(objpfx)tst-unique2.out: $(objpfx)tst-unique2mod2.so

$(objpfx)tst-unique3: $(libdl) $(objpfx)tst-unique3lib.so
$(objpfx)tst-unique3.out: $(objpfx)tst-unique3lib2.so

$(objpfx)tst-unique4: $(objpfx)tst-unique4lib.so

$(objpfx)tst-nodelete: $(libdl)
$(objpfx)tst-nodelete.out: $(objpfx)tst-nodelete-uniquemod.so \
			   $(objpfx)tst-nodelete-rtldmod.so \
			   $(objpfx)tst-nodelete-zmod.so

LDFLAGS-tst-nodelete = -rdynamic
LDFLAGS-tst-nodelete-zmod.so = -Wl,--enable-new-dtags,-z,nodelete

$(objpfx)tst-nodelete2: $(libdl)
$(objpfx)tst-nodelete2.out: $(objpfx)tst-nodelete2mod.so

LDFLAGS-tst-nodelete2 = -rdynamic

$(objpfx)tst-initorder-cmp.out: tst-initorder.exp $(objpfx)tst-initorder.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-initorder2: $(objpfx)tst-initorder2a.so $(objpfx)tst-initorder2d.so $(objpfx)tst-initorder2c.so
$(objpfx)tst-initorder2a.so: $(objpfx)tst-initorder2b.so
$(objpfx)tst-initorder2b.so: $(objpfx)tst-initorder2c.so
$(objpfx)tst-initorder2c.so: $(objpfx)tst-initorder2d.so
LDFLAGS-tst-initorder2 = $(no-as-needed)
LDFLAGS-tst-initorder2a.so = $(no-as-needed)
LDFLAGS-tst-initorder2b.so = $(no-as-needed)
LDFLAGS-tst-initorder2c.so = $(no-as-needed)
define o-iterator-doit
$(objpfx)tst-initorder2$o.os: tst-initorder2.c; \
$$(compile-command.c) -DNAME=\"$o\"
endef
object-suffixes-left := a b c d
include $(o-iterator)

$(objpfx)tst-initorder2-cmp.out: tst-initorder2.exp $(objpfx)tst-initorder2.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-relsort1: $(libdl)
$(objpfx)tst-relsort1mod1.so: $(libm) $(objpfx)tst-relsort1mod2.so
$(objpfx)tst-relsort1mod2.so: $(libm)
$(objpfx)tst-relsort1.out: $(objpfx)tst-relsort1mod1.so \
			   $(objpfx)tst-relsort1mod2.so

$(objpfx)tst-unused-dep.out: $(objpfx)testobj1.so
	$(test-wrapper-env) \
	LD_TRACE_LOADED_OBJECTS=1 \
	LD_DEBUG=unused \
	LD_PRELOAD= \
	$(rtld-prefix) \
	  $< > $@; \
	$(evaluate-test)

$(objpfx)tst-unused-dep-cmp.out: $(objpfx)tst-unused-dep.out
	cmp $< /dev/null > $@; \
	$(evaluate-test)

$(objpfx)tst-audit11.out: $(objpfx)tst-auditmod11.so $(objpfx)tst-audit11mod1.so
$(objpfx)tst-audit11: $(libdl)
tst-audit11-ENV = LD_AUDIT=$(objpfx)tst-auditmod11.so
$(objpfx)tst-audit11mod1.so: $(objpfx)tst-audit11mod2.so
LDFLAGS-tst-audit11mod2.so = -Wl,--version-script=tst-audit11mod2.map,-soname,tst-audit11mod2.so

$(objpfx)tst-audit12.out: $(objpfx)tst-auditmod12.so $(objpfx)tst-audit12mod1.so $(objpfx)tst-audit12mod3.so
$(objpfx)tst-audit12: $(libdl)
tst-audit12-ENV = LD_AUDIT=$(objpfx)tst-auditmod12.so
$(objpfx)tst-audit12mod1.so: $(objpfx)tst-audit12mod2.so
LDFLAGS-tst-audit12mod2.so = -Wl,--version-script=tst-audit12mod2.map

# Override -z defs, so that we can reference an undefined symbol.
# Force lazy binding for the same reason.
LDFLAGS-tst-latepthreadmod.so = \
  -Wl,-z,lazy -Wl,--unresolved-symbols=ignore-all
# Do not optimize sibling calls as the test relies on a JMP_SLOT relocation for
# function this_function_is_not_defined.
CFLAGS-tst-latepthreadmod.c += -fno-optimize-sibling-calls
$(objpfx)tst-latepthreadmod.so: $(shared-thread-library)
$(objpfx)tst-latepthread: $(libdl)
$(objpfx)tst-latepthread.out: $(objpfx)tst-latepthreadmod.so

# The test modules are parameterized by preprocessor macros.
$(patsubst %,$(objpfx)%.os,$(tst-tls-many-dynamic-modules)): \
  $(objpfx)tst-tls-manydynamic%mod.os : tst-tls-manydynamicmod.c
	$(compile-command.c) \
	  -DNAME=tls_global_$* -DSETTER=set_value_$* -DGETTER=get_value_$*
$(objpfx)tst-tls-manydynamic: $(libdl) $(shared-thread-library)
$(objpfx)tst-tls-manydynamic.out: \
  $(patsubst %,$(objpfx)%.so,$(tst-tls-many-dynamic-modules))

tst-prelink-ENV = LD_TRACE_PRELINKING=1

$(objpfx)tst-prelink-conflict.out: $(objpfx)tst-prelink.out
	grep stdout $< | grep conflict | $(AWK) '{ print $$10, $$11 }' > $@

$(objpfx)tst-prelink-cmp.out: tst-prelink.exp \
			      $(objpfx)tst-prelink-conflict.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-ldconfig-X.out : tst-ldconfig-X.sh $(objpfx)ldconfig
	$(SHELL) $< '$(common-objpfx)' '$(test-wrapper-env)' \
		 '$(run-program-env)' > $@; \
	$(evaluate-test)

$(objpfx)tst-dlsym-error: $(libdl)

# Test static linking of all the libraries we can possibly link
# together.  Note that in some configurations this may be less than the
# complete list of libraries we build but we try to maxmimize this list.
$(objpfx)tst-linkall-static: \
  $(common-objpfx)math/libm.a \
  $(common-objpfx)resolv/libresolv.a \
  $(common-objpfx)dlfcn/libdl.a \
  $(common-objpfx)login/libutil.a \
  $(common-objpfx)rt/librt.a \
  $(common-objpfx)resolv/libanl.a \
  $(static-thread-library)

ifeq ($(build-crypt),yes)
# If we are using NSS crypto and we have the ability to link statically
# then we include libcrypt.a, otherwise we leave out libcrypt.a and
# link as much as we can into the tst-linkall-static test.  This assumes
# that linking with libcrypt.a does everything required to include the
# static NSS crypto library.
ifeq (yesyes,$(nss-crypt)$(static-nss-crypt))
$(objpfx)tst-linkall-static: \
  $(common-objpfx)crypt/libcrypt.a
endif
# If we are not using NSS crypto then we always have the ability to link
# with libcrypt.a.
ifeq (no,$(nss-crypt))
$(objpfx)tst-linkall-static: \
  $(common-objpfx)crypt/libcrypt.a
endif
endif

# The application depends on the DSO, and the DSO loads the plugin.
# The plugin also depends on the DSO. This creates the circular
# dependency via dlopen that we're testing to make sure works.
$(objpfx)tst-nodelete-dlclose-dso.so: $(libdl)
$(objpfx)tst-nodelete-dlclose-plugin.so: $(objpfx)tst-nodelete-dlclose-dso.so
$(objpfx)tst-nodelete-dlclose: $(objpfx)tst-nodelete-dlclose-dso.so
$(objpfx)tst-nodelete-dlclose.out: $(objpfx)tst-nodelete-dlclose-dso.so \
				   $(objpfx)tst-nodelete-dlclose-plugin.so

tst-env-setuid-ENV = MALLOC_CHECK_=2 MALLOC_MMAP_THRESHOLD_=4096 \
		     LD_HWCAP_MASK=0x1
tst-env-setuid-tunables-ENV = \
	GLIBC_TUNABLES=glibc.malloc.check=2:glibc.malloc.mmap_threshold=4096

$(objpfx)tst-debug1: $(libdl)
$(objpfx)tst-debug1.out: $(objpfx)tst-debug1mod1.so

$(objpfx)tst-debug1mod1.so: $(objpfx)testobj1.so
	$(OBJCOPY) --only-keep-debug $< $@

$(objpfx)tst-main1: $(objpfx)tst-main1mod.so
CRT-tst-main1 := $(csu-objpfx)crt1.o
tst-main1-no-pie = yes
LDLIBS-tst-main1 = $(libsupport)
tst-main1mod.so-no-z-defs = yes

LDLIBS-tst-absolute-sym-lib.so = tst-absolute-sym-lib.lds
$(objpfx)tst-absolute-sym-lib.so: $(LDLIBS-tst-absolute-sym-lib.so)
$(objpfx)tst-absolute-sym: $(objpfx)tst-absolute-sym-lib.so

LDLIBS-tst-absolute-zero-lib.so = tst-absolute-zero-lib.lds
$(objpfx)tst-absolute-zero-lib.so: $(LDLIBS-tst-absolute-zero-lib.so)
$(objpfx)tst-absolute-zero: $(objpfx)tst-absolute-zero-lib.so

# Both the main program and the DSO for tst-libc_dlvsym need to link
# against libdl.
$(objpfx)tst-libc_dlvsym: $(libdl)
$(objpfx)tst-libc_dlvsym-dso.so: $(libsupport) $(libdl)
$(objpfx)tst-libc_dlvsym.out: $(objpfx)tst-libc_dlvsym-dso.so
$(objpfx)tst-libc_dlvsym-static: $(common-objpfx)dlfcn/libdl.a
tst-libc_dlvsym-static-ENV = \
  LD_LIBRARY_PATH=$(objpfx):$(common-objpfx):$(common-objpfx)dlfcn
$(objpfx)tst-libc_dlvsym-static.out: $(objpfx)tst-libc_dlvsym-dso.so

$(objpfx)tst-big-note: $(objpfx)tst-big-note-lib.so
