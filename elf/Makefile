# Copyright (C) 1995-1999, 2000, 2001 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

# Makefile for elf subdirectory of GNU C Library.

subdir		:= elf

headers		= elf.h bits/elfclass.h link.h
routines	= $(dl-routines) dl-open dl-close dl-support \
		  dl-addr enbl-secure dl-profstub dl-origin dl-libc dl-sym

# The core dynamic linking functions are in libc for the static and
# profiled libraries.
dl-routines	= $(addprefix dl-,load cache lookup object reloc deps \
			          runtime error init fini debug misc \
				  version profile)
all-dl-routines = $(dl-routines) $(sysdep-dl-routines)
# But they are absent from the shared libc, because that code is in ld.so.
elide-routines.os = $(all-dl-routines) dl-support enbl-secure

# ld.so uses those routines, plus some special stuff for being the program
# interpreter and operating independent of libc.
rtld-routines	:= rtld $(dl-routines) dl-sysdep dl-environ dl-minimal
all-rtld-routines = $(rtld-routines) $(sysdep-rtld-routines)

distribute	:= $(rtld-routines:=.c) dynamic-link.h do-rel.h dl-machine.h \
		   dl-cache.h dl-hash.h soinit.c sofini.c ldd.bash.in \
		   genrtldtbl.awk atomicity.h dl-procinfo.h ldsodefs.h \
		   dl-librecon.h interp.c sln.c dl-dst.h hp-timing.h \
		   do-lookup.h dl-lookupcfg.h sprof.c gen-trusted-dirs.awk \
		   testobj1.c testobj2.c testobj3.c testobj4.c testobj5.c \
		   testobj6.c testobj1_1.c failobj.c unloadmod.c \
		   ldconfig.h ldconfig.c cache.c readlib.c readelflib.c \
		   chroot_canon.c gccframe.h \
		   dep1.c dep2.c dep3.c dep4.c dl-dtprocnum.h unsecvars.h \
		   vismain.c vismod1.c vismod2.c vismod3.c \
		   constload2.c constload3.c filtmod1.c filtmod2.c \
		   nodlopenmod.c nodelete.c nodelmod1.c nodelmod2.c \
		   nodelmod3.c nodelmod4.c nodlopen.c dl-osinfo.h \
		   reldepmod1.c reldepmod2.c reldepmod3.c reldepmod4.c \
		   reldep4mod1.c reldep4mod2.c reldep4mod3.c reldep4mod4.c \
		   nextmod1.c nextmod2.c pathoptobj.c tst-pathopt.sh \
		   neededobj1.c neededobj2.c neededobj3.c neededobj4.c \
		   neededobj5.c neededobj6.c firstobj.c \
		   unload2mod.c unload2dep.c ltglobmod1.c ltglobmod2.c \
		   testobj.h vismod.h globalmod1.c

include ../Makeconfig

before-compile  = $(objpfx)trusted-dirs.h
generated	:= trusted-dirs.h trusted-dirs.st for-renamed/renamed.so
generated-dirs	:= for-renamed

ifeq ($(versioning),yes)
ld-map		= $(common-objpfx)ld.map
endif

ifeq (yes,$(build-shared))
extra-objs	= $(all-rtld-routines:%=%.os) soinit.os sofini.os interp.os
generated	+= librtld.os dl-allobjs.os ld.so ldd
install-others	= $(inst_slibdir)/$(rtld-installed-name)
install-bin	= ldd
endif

others		= sprof sln
install-bin	+= sprof
others-static   = sln
install-rootsbin = sln

ifeq (yes,$(use-ldconfig))
ifeq (yes,$(build-shared))
others-static	+= ldconfig
others		+= ldconfig
install-rootsbin += ldconfig

ldconfig-modules := cache readlib xmalloc xstrdup chroot_canon
extra-objs	+= $(ldconfig-modules:=.o)

# To find xmalloc.c and xstrdup.c
vpath %.c ../locale/programs

endif
endif

ifeq (yes,$(build-shared))
tests = loadtest restest1 preloadtest loadfail multiload origtest resolvfail \
	constload1 order $(tests-vis-$(have-protected)) noload filter unload \
	reldep reldep2 reldep3 reldep4 next $(tests-nodelete-$(have-z-nodelete)) \
	$(tests-nodlopen-$(have-z-nodlopen)) neededtest neededtest2 \
	neededtest3 neededtest4 unload2 lateglobal initfirst global \
	restest2
test-srcs = tst-pathopt
tests-vis-yes = vismain
tests-nodelete-yes = nodelete
tests-nodlopen-yes = nodlopen
endif
modules-names = testobj1 testobj2 testobj3 testobj4 testobj5 testobj6 \
		testobj1_1 failobj constload2 constload3 unloadmod \
		dep1 dep2 dep3 dep4 $(modules-vis-$(have-protected)) \
		$(modules-nodelete-$(have-z-nodelete)) \
		$(modules-nodlopen-$(have-z-nodlopen)) filtmod1 filtmod2 \
		reldepmod1 reldepmod2 reldepmod3 reldepmod4 nextmod1 nextmod2 \
		reldep4mod1 reldep4mod2 reldep4mod3 reldep4mod4 \
		neededobj1 neededobj2 neededobj3 neededobj4 \
		neededobj5 neededobj6 firstobj globalmod1 \
		unload2mod unload2dep ltglobmod1 ltglobmod2 pathoptobj
modules-vis-yes = vismod1 vismod2 vismod3
modules-nodelete-yes = nodelmod1 nodelmod2 nodelmod3 nodelmod4
modules-nodlopen-yes = nodlopenmod
extra-objs += $(addsuffix .os,$(strip $(modules-names)))

include ../Rules

ifeq (yes,$(build-shared))
# Make sure these things are built in the `make lib' pass so they can be used
# to run programs during the `make others' pass.
lib-noranlib: $(objpfx)$(rtld-installed-name) \
	      $(addprefix $(objpfx),$(extra-objs))
endif

# Command to link into a larger single relocatable object.
reloc-link = $(LINK.o) -nostdlib -nostartfiles -r -o $@

$(objpfx)dl-allobjs.os: $(all-rtld-routines:%=$(objpfx)%.os)
	$(reloc-link) $^

# Link together the dynamic linker into a single relocatable object.
$(objpfx)librtld.os: $(objpfx)dl-allobjs.os $(common-objpfx)libc_pic.a
	$(reloc-link) '-Wl,-(' $^ -lgcc '-Wl,-)'

# Do we need a linker script?
rtld-ldscript-in := $(firstword $(wildcard $(+sysdep_dirs:%=%/rtld-ldscript.in)))

ifneq (,$(rtld-ldscript-in))
rtld-ldscript = $(objpfx)rtld-ldscript
generated += rtld-ldscript

LDFLAGS-rtld = -T $(rtld-ldscript)
before-compile += $(rtld-ldscript)

rtld-parms = $(wildcard $(+sysdep_dirs:%=%/rtld-parms))
include $(rtld-parms)

$(rtld-ldscript): $(rtld-ldscript-in) $(rtld-parms)
	sed -e 's#@@rtld-oformat@@#$(rtld-oformat)#' \
	    -e 's#@@rtld-arch@@#$(rtld-arch)#' \
	    -e 's#@@rtld-entry@@#$(rtld-entry)#' \
	    -e 's#@@rtld-base@@#$(rtld-base)#' $< >$@
endif

$(objpfx)ld.so: $(objpfx)librtld.os $(rtld-ldscript) $(ld-map)
	$(LINK.o) -nostdlib -nostartfiles -shared -o $@ $(LDFLAGS-rtld) \
		  $(filter-out $(rtld-ldscript) $(map-file),$^)		\
		  $(load-map-file) -Wl,-soname=$(rtld-installed-name)

# interp.c exists just to get this string into the libraries.
CFLAGS-interp.c = -D'RUNTIME_LINKER="$(slibdir)/$(rtld-installed-name)"'

ifneq (ld.so,$(rtld-installed-name))
# Make sure ld.so.1 exists in the build directory so we can link
# against it.
$(objpfx)$(rtld-installed-name): $(objpfx)ld.so
	rm -f $@
	ln -s $(<F) $@
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
	$(move-if-change) ${@:st=T} ${@:st=h}
	touch $@
CPPFLAGS-dl-load.c = -I$(objpfx). -I$(csu-objpfx).

ifeq (yes,$(build-shared))
$(inst_slibdir)/$(rtld-version-installed-name): $(objpfx)ld.so $(+force)
	$(make-target-directory)
	$(do-install-program)

$(inst_slibdir)/$(rtld-installed-name): \
  $(inst_slibdir)/$(rtld-version-installed-name) \
  $(inst_slibdir)/libc-$(version).so
	$(make-shlib-link)

# Special target called by parent to install just the dynamic linker.
.PHONY: ldso_install
ldso_install: $(inst_slibdir)/$(rtld-installed-name)
endif


common-ldd-rewrite = -e 's%@RTLD@%$(slibdir)/$(rtld-installed-name)%g' \
		     -e 's%@VERSION@%$(version)%g'
sh-ldd-rewrite = $(common-ldd-rewrite) -e 's%@BASH@%/bin/sh%g;s/\$$"/"/g'
bash-ldd-rewrite = $(common-ldd-rewrite) -e 's%@BASH@%$(BASH)%g' \
		   -e 's%@TEXTDOMAINDIR@%$(msgcatdir)%g'

ifneq ($(have-bash2),yes)
ldd-shell = sh
else
ldd-shell = bash
endif

ifeq ($(ldd-rewrite-script),no)
define gen-ldd
sed $($(ldd-shell)-ldd-rewrite) < $< > $@.new
endef
else
define gen-ldd
sed $($(ldd-shell)-ldd-rewrite) < $< | sed -f $(ldd-rewrite-script) > $@.new
endef
endif

$(objpfx)ldd: ldd.bash.in $(common-objpfx)soversions.mk \
	      $(common-objpfx)config.make
	$(gen-ldd)
	chmod 555 $@.new
	mv -f $@.new $@

$(objpfx)sprof: $(libdl)

$(objpfx)ldconfig: $(ldconfig-modules:%=$(objpfx)%.o)
SYSCONF-FLAGS := -D'SYSCONFDIR="$(sysconfdir)"'
CFLAGS-ldconfig.c = $(SYSCONF-FLAGS) -D'LIBDIR="$(libdir)"' -D'SLIBDIR="$(slibdir)"'
CFLAGS-dl-cache.c = $(SYSCONF-FLAGS)
CFLAGS-cache.c = $(SYSCONF-FLAGS)

test-modules = $(addprefix $(objpfx),$(addsuffix .so,$(strip $(modules-names))))
generated += $(addsuffix .so,$(strip $(modules-names)))

ifeq (yes,$(build-shared))
ifeq ($(cross-compiling),no)
tests: $(objpfx)tst-pathopt.out
endif
endif

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

# filtmod1.so has a special rule
$(filter-out $(objpfx)filtmod1.so, $(test-modules)): $(objpfx)%.so: $(objpfx)%.os
	$(build-module)

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

neededtest4-ENV = LC_ALL=C LANGUAGE=C
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
CFLAGS-multiload.c = -DOBJDIR=\"$(elf-objpfx)\"

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

$(objpfx)order: $(addprefix $(objpfx),dep4.so dep3.so dep2.so dep1.so)

$(objpfx)order.out: $(objpfx)order
	$(elf-objpfx)$(rtld-installed-name) \
	  --library-path $(rpath-link)$(patsubst %,:%,$(sysdep-library-path)) \
	  $(objpfx)order > $@
	(echo "0123456789" | cmp $@ -) > /dev/null

$(objpfx)vismain: $(addprefix $(objpfx),vismod1.so vismod2.so)
$(objpfx)vismain.out: $(addprefix $(objpfx),vismod3.so)
vismain-ENV = LD_PRELOAD=$(addprefix $(objpfx),vismod3.so)

$(objpfx)noload: $(objpfx)testobj1.so
LDFLAGS-noload = -rdynamic
$(objpfx)noload.out: $(objpfx)testobj5.so

LDFLAGS-nodelete = -rdynamic
LDFLAGS-nodelmod1.so = -Wl,--enable-new-dtags,-z,nodelete
LDFLAGS-nodelmod4.so = -Wl,--enable-new-dtags,-z,nodelete
$(objpfx)nodelete: $(libdl)
$(objpfx)nodelete.out: $(objpfx)nodelmod1.so $(objpfx)nodelmod2.so \
		       $(objpfx)nodelmod3.so

LDFLAGS-nodlopenmod.so = -Wl,--enable-new-dtags,-z,nodlopen
$(objpfx)nodlopen: $(libdl)
$(objpfx)nodlopen.out: $(objpfx)nodlopenmod.so

$(objpfx)filtmod1.so: $(objpfx)filtmod1.os $(objpfx)filtmod2.so
	$(LINK.o) -shared -o $@ -B$(csu-objpfx) $(LDFLAGS.so) \
		  -L$(subst :, -L,$(rpath-link)) \
		  -Wl,-rpath-link=$(rpath-link) \
		  $< -Wl,-F,$(objpfx)filtmod2.so
$(objpfx)filter: $(objpfx)filtmod1.so

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

$(objpfx)unload2: $(libdl)
$(objpfx)unload2.out: $(objpfx)unload2mod.so $(objpfx)unload2dep.so

$(objpfx)lateglobal: $(libdl)
$(objpfx)lateglobal.out: $(objpfx)ltglobmod1.so $(objpfx)ltglobmod2.so

$(objpfx)tst-pathopt: $(libdl)
$(objpfx)tst-pathopt.out: tst-pathopt.sh $(objpfx)tst-pathopt \
			  $(objpfx)pathoptobj.so
	$(SHELL) -e $< $(common-objpfx)

$(objpfx)initfirst: $(libdl)
$(objpfx)initfirst.out: $(objpfx)firstobj.so

$(objpfx)global: $(objpfx)globalmod1.so
$(objpfx)global.out: $(objpfx)reldepmod1.so
