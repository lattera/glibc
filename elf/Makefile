# Copyright (C) 1995-2007, 2008 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

# Makefile for elf subdirectory of GNU C Library.

subdir		:= elf

headers		= elf.h bits/elfclass.h link.h bits/link.h
routines	= $(dl-routines) dl-support dl-iteratephdr \
		  dl-addr enbl-secure dl-profstub \
		  dl-origin dl-libc dl-sym dl-tsd dl-sysdep

# The core dynamic linking functions are in libc for the static and
# profiled libraries.
dl-routines	= $(addprefix dl-,load cache lookup object reloc deps \
			          runtime error init fini debug misc \
				  version profile conflict tls origin \
				  execstack caller open close trampoline)
all-dl-routines = $(dl-routines) $(sysdep-dl-routines)
# But they are absent from the shared libc, because that code is in ld.so.
elide-routines.os = $(all-dl-routines) dl-support enbl-secure dl-origin \
		    dl-sysdep
shared-only-routines += dl-caller

# ld.so uses those routines, plus some special stuff for being the program
# interpreter and operating independent of libc.
rtld-routines	:= rtld $(dl-routines) dl-sysdep dl-environ dl-minimal
all-rtld-routines = $(rtld-routines) $(sysdep-rtld-routines)

distribute	:= rtld-Rules \
		   $(rtld-routines:=.c) dynamic-link.h do-rel.h dl-machine.h \
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
		   reldepmod5.c reldepmod6.c \
		   reldep4mod1.c reldep4mod2.c reldep4mod3.c reldep4mod4.c \
		   nextmod1.c nextmod2.c pathoptobj.c tst-pathopt.sh \
		   neededobj1.c neededobj2.c neededobj3.c neededobj4.c \
		   neededobj5.c neededobj6.c firstobj.c \
		   unload2mod.c unload2dep.c ltglobmod1.c ltglobmod2.c \
		   testobj.h vismod.h globalmod1.c \
		   dblloadmod1.c dblloadmod2.c dblloadmod3.c \
		   reldep6mod4.c reldep6mod3.c reldep6mod2.c reldep6mod1.c \
		   reldep6mod0.c reldep7mod1.c reldep7mod2.c \
		   unwind-dw2.c unwind-dw2-fde.c unwind.h unwind-pe.h \
		   unwind-dw2-fde.h dwarf2.h dl-procinfo.c tls.h dl-tls.h \
		   tst-tlsmod1.c tst-tlsmod2.c tst-tlsmod3.c tst-tlsmod4.c \
		   tst-tlsmod5.c tst-tlsmod6.c tst-tlsmod7.c tst-tlsmod8.c \
		   tst-tlsmod9.c tst-tlsmod10.c tst-tlsmod11.c \
		   tst-tlsmod12.c tst-tls10.h tst-alignmod.c tst-alignmod2.c \
		   circlemod1.c circlemod1a.c circlemod2.c circlemod2a.c \
		   circlemod3.c circlemod3a.c nodlopenmod2.c \
		   tls-macros.h \
		   reldep8mod1.c reldep8mod2.c reldep8mod3.c \
		   nodel2mod1.c nodel2mod2.c nodel2mod3.c \
		   reldep9.c reldep9mod1.c reldep9mod2.c reldep9mod3.c \
		   tst-array1.exp tst-array2.exp tst-array4.exp \
		   tst-array2dep.c tst-piemod1.c \
		   tst-execstack-mod.c tst-dlmodcount.c \
		   check-textrel.c dl-sysdep.h test-dlopenrpathmod.c \
		   tst-deep1mod1.c tst-deep1mod2.c tst-deep1mod3.c \
		   unload3mod1.c unload3mod2.c unload3mod3.c unload3mod4.c \
		   unload4mod1.c unload4mod2.c unload4mod3.c unload4mod4.c \
		   unload6mod1.c unload6mod2.c unload6mod3.c \
		   unload7mod1.c unload7mod2.c \
		   tst-auditmod1.c tst-audit.sh \
		   order2mod1.c order2mod2.c order2mod3.c order2mod4.c \
		   tst-stackguard1.c tst-stackguard1-static.c \
		   tst-array5.c tst-array5-static.c tst-array5dep.c \
		   tst-array5.exp tst-leaks1.c

CFLAGS-dl-runtime.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-dl-lookup.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-dl-iterate-phdr.c = $(uses-callbacks)

include ../Makeconfig

ifeq ($(unwind-find-fde),yes)
routines += unwind-dw2-fde-glibc
shared-only-routines += unwind-dw2-fde-glibc
endif

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
install-bin-script = ldd
endif

others		= sprof sln
install-bin	= sprof
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

tests = tst-tls1 tst-tls2 tst-tls9 tst-leaks1
ifeq (yes,$(have-initfini-array))
tests += tst-array1 tst-array2 tst-array3 tst-array4 tst-array5
endif
ifeq (yes,$(build-static))
tests-static = tst-tls1-static tst-tls2-static tst-stackguard1-static
ifeq (yesyesyes,$(build-static)$(build-shared)$(elf))
tests-static += tst-tls9-static
tst-tls9-static-ENV = \
       LD_LIBRARY_PATH=$(objpfx):$(common-objpfx):$(common-objpfx)dlfcn
endif
ifeq (yes,$(have-initfini-array))
tests-static += tst-array1-static tst-array5-static
endif
tests += $(tests-static)
endif
ifeq (yes,$(build-shared))
tests += loadtest restest1 preloadtest loadfail multiload origtest resolvfail \
	 constload1 order vismain noload filter unload \
	 reldep reldep2 reldep3 reldep4 nodelete nodelete2 \
	 nodlopen nodlopen2 neededtest neededtest2 \
	 neededtest3 neededtest4 unload2 lateglobal initfirst global \
	 restest2 next dblload dblunload reldep5 reldep6 reldep7 reldep8 \
	 circleload1 tst-tls3 tst-tls4 tst-tls5 tst-tls6 tst-tls7 tst-tls8 \
	 tst-tls10 tst-tls11 tst-tls12 tst-tls13 tst-tls14 tst-tls15 \
	 tst-tls16 tst-tls17 tst-tls18 tst-tls-dlinfo \
	 tst-align tst-align2 $(tests-execstack-$(have-z-execstack)) \
	 tst-dlmodcount tst-dlopenrpath tst-deep1 \
	 tst-dlmopen1 tst-dlmopen2 tst-dlmopen3 \
	 unload3 unload4 unload5 unload6 unload7 tst-global1 order2 \
	 tst-audit1 tst-audit2 \
	 tst-stackguard1 tst-addr1 tst-thrlock
#	 reldep9
test-srcs = tst-pathopt
tests-execstack-yes = tst-execstack tst-execstack-needed tst-execstack-prog
endif
ifeq (yesyes,$(have-fpie)$(build-shared))
tests: $(objpfx)tst-pie1.out
endif
tests: $(objpfx)tst-leaks1-mem
tlsmod17a-suffixes = 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
tlsmod18a-suffixes = 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
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
		$(patsubst %,tst-tlsmod17a%,$(tlsmod17a-suffixes)) \
		tst-tlsmod17b \
		$(patsubst %,tst-tlsmod18a%,$(tlsmod18a-suffixes)) \
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
		order2mod1 order2mod2 order2mod3 order2mod4
ifeq (yes,$(have-initfini-array))
modules-names += tst-array2dep tst-array5dep
endif
ifeq (yesyes,$(have-fpie)$(build-shared))
modules-names += tst-piemod1
endif
modules-execstack-yes = tst-execstack-mod
extra-test-objs += $(addsuffix .os,$(strip $(modules-names)))
# We need this variable to be sure the test modules get the right CPPFLAGS.
test-extras += $(modules-names)

# filtmod1.so has a special rule
modules-names-nobuild := filtmod1


include ../Rules

check-abi: check-abi-ld
update-abi: update-abi-ld

ifeq (yes,$(build-shared))
# Make sure these things are built in the `make lib' pass so they can be used
# to run programs during the `make others' pass.
lib-noranlib: $(objpfx)$(rtld-installed-name) \
	      $(addprefix $(objpfx),$(extra-objs))
endif

# Command to link into a larger single relocatable object.
reloc-link = $(LINK.o) -nostdlib -nostartfiles -r

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

$(objpfx)librtld.map: $(objpfx)dl-allobjs.os $(common-objpfx)libc_pic.a
	@-rm -f $@T
	$(reloc-link) -o $@.o '-Wl,-(' $^ -lgcc '-Wl,-)' -Wl,-Map,$@T
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
	@rm -f $@.lds
	$(LINK.o) -nostdlib -nostartfiles -shared $(z-now-$(bind-now))	\
		  $(LDFLAGS-rtld) -Wl,-z,defs -Wl,--verbose 2>&1 |	\
		  LC_ALL=C \
		  sed -e '/^=========/,/^=========/!d;/^=========/d'	\
		      -e 's/\. = 0 + SIZEOF_HEADERS;/& _begin = . - SIZEOF_HEADERS;/' \
		  > $@.lds
	$(LINK.o) -nostdlib -nostartfiles -shared -o $@			\
		  $(LDFLAGS-rtld) -Wl,-z,defs $(z-now-$(bind-now))	\
		  $(filter-out $(map-file),$^) $(load-map-file)		\
		  -Wl,-soname=$(rtld-installed-name) -T $@.lds
	rm -f $@.lds
	readelf -s $@ \
	  | $(AWK) '($$7 ~ /^UND(|EF)$$/ && $$1 != "0:" && $$4 != "REGISTER") { print; p=1 } END { exit p != 0 }'

# interp.c exists just to get this string into the libraries.
CFLAGS-interp.c = -D'RUNTIME_LINKER="$(slibdir)/$(rtld-installed-name)"' \
		  -DNOT_IN_libc=1
$(objpfx)interp.os: $(common-objpfx)config.make

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
	echo '#define DL_DST_LIB "$(notdir $(slibdir))"' >> ${@:st=T}
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
LC_ALL=C sed $($(ldd-shell)-ldd-rewrite) < $< > $@.new
endef
else
define gen-ldd
LC_ALL=C sed $($(ldd-shell)-ldd-rewrite) < $< | LC_ALL=C sed -f $(ldd-rewrite-script) > $@.new
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
CFLAGS-ldconfig.c = $(SYSCONF-FLAGS) -D'LIBDIR="$(libdir)"' \
		    -D'SLIBDIR="$(slibdir)"' -DIS_IN_ldconfig=1
CFLAGS-dl-cache.c = $(SYSCONF-FLAGS)
CFLAGS-cache.c = $(SYSCONF-FLAGS)

CPPFLAGS-.os += $(if $(filter $(@F),$(patsubst %,%.os,$(all-rtld-routines))),-DNOT_IN_libc=1 -DIS_IN_rtld=1)

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

LDFLAGS-tst-tlsmod5.so = -nostdlib
LDFLAGS-tst-tlsmod6.so = -nostdlib

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

$(objpfx)circleload1: $(libdl)
$(objpfx)circleload1.out: $(objpfx)circlemod1.so \
			  $(objpfx)circlemod1a.so

$(objpfx)circlemod1.so: $(objpfx)circlemod2.so
$(objpfx)circlemod2.so: $(objpfx)circlemod3.so
$(objpfx)circlemod1a.so: $(objpfx)circlemod2a.so
$(objpfx)circlemod2a.so: $(objpfx)circlemod3a.so

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

$(objpfx)nodlopenmod2.so: $(objpfx)nodlopenmod.so
$(objpfx)nodlopen2: $(libdl)
$(objpfx)nodlopen2.out: $(objpfx)nodlopenmod2.so

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

$(objpfx)tst-tls10: $(objpfx)tst-tlsmod8.so

$(objpfx)tst-tls11: $(objpfx)tst-tlsmod10.so

$(objpfx)tst-tls12: $(objpfx)tst-tlsmod12.so

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
$(patsubst %,$(objpfx)tst-tlsmod17a%.os,$(tlsmod17a-suffixes)): $(objpfx)tst-tlsmod17a%.os : tst-tlsmod17a.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ -DN=$* -DNOT_IN_libc=1 $<
$(patsubst %,$(objpfx)tst-tlsmod17a%.so,$(tlsmod17a-suffixes)): $(objpfx)tst-tlsmod17a%.so: $(objpfx)ld.so
$(objpfx)tst-tlsmod17b.so: $(patsubst %,$(objpfx)tst-tlsmod17a%.so,$(tlsmod17a-suffixes))

$(objpfx)tst-tls18: $(libdl)
$(objpfx)tst-tls18.out: $(patsubst %,$(objpfx)tst-tlsmod18a%.so,$(tlsmod18a-suffixes))
$(patsubst %,$(objpfx)tst-tlsmod18a%.os,$(tlsmod18a-suffixes)): $(objpfx)tst-tlsmod18a%.os : tst-tlsmod18a.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ -DN=$* -DNOT_IN_libc=1 $<
$(patsubst %,$(objpfx)tst-tlsmod18a%.so,$(tlsmod18a-suffixes)): $(objpfx)tst-tlsmod18a%.so: $(objpfx)ld.so

CFLAGS-tst-align.c = $(stack-align-test-flags)
CFLAGS-tst-align2.c = $(stack-align-test-flags)
CFLAGS-tst-alignmod.c = $(stack-align-test-flags)
CFLAGS-tst-alignmod2.c = $(stack-align-test-flags)
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

ifdef libdl
$(objpfx)tst-tls9-static: $(common-objpfx)dlfcn/libdl.a
$(objpfx)tst-tls9-static.out: $(objpfx)tst-tlsmod5.so $(objpfx)tst-tlsmod6.so
endif

ifeq ($(have-z-execstack),yes)
$(objpfx)tst-execstack: $(libdl)
$(objpfx)tst-execstack.out: $(objpfx)tst-execstack-mod.so
LDFLAGS-tst-execstack = -Wl,-z,noexecstack
LDFLAGS-tst-execstack-mod = -Wl,-z,execstack

$(objpfx)tst-execstack-needed: $(objpfx)tst-execstack-mod.so
LDFLAGS-tst-execstack-needed = -Wl,-z,noexecstack

LDFLAGS-tst-execstack-prog = -Wl,-z,execstack
endif

$(objpfx)tst-array1.out: $(objpfx)tst-array1
	$(elf-objpfx)$(rtld-installed-name) \
	  --library-path $(rpath-link)$(patsubst %,:%,$(sysdep-library-path)) \
	  $(objpfx)tst-array1 > $@
	cmp $@ tst-array1.exp > /dev/null

$(objpfx)tst-array1-static.out: $(objpfx)tst-array1-static
	$(objpfx)tst-array1-static > $@
	cmp $@ tst-array1.exp > /dev/null

$(objpfx)tst-array2: $(objpfx)tst-array2dep.so
$(objpfx)tst-array2.out: $(objpfx)tst-array2
	$(elf-objpfx)$(rtld-installed-name) \
	  --library-path $(rpath-link)$(patsubst %,:%,$(sysdep-library-path)) \
	  $(objpfx)tst-array2 > $@
	cmp $@ tst-array2.exp > /dev/null

$(objpfx)tst-array3.out: $(objpfx)tst-array3
	$(elf-objpfx)$(rtld-installed-name) \
	  --library-path $(rpath-link)$(patsubst %,:%,$(sysdep-library-path)) \
	  $(objpfx)tst-array3 > $@
	cmp $@ tst-array1.exp > /dev/null

$(objpfx)tst-array4: $(libdl)
$(objpfx)tst-array4.out: $(objpfx)tst-array4 $(objpfx)tst-array2dep.so
	$(elf-objpfx)$(rtld-installed-name) \
	  --library-path $(rpath-link)$(patsubst %,:%,$(sysdep-library-path)) \
	  $< > $@
	cmp $@ tst-array4.exp > /dev/null

$(objpfx)tst-array5: $(objpfx)tst-array5dep.so
$(objpfx)tst-array5.out: $(objpfx)tst-array5
	$(elf-objpfx)$(rtld-installed-name) \
	  --library-path $(rpath-link)$(patsubst %,:%,$(sysdep-library-path)) \
	  $(objpfx)tst-array5 > $@
	cmp $@ tst-array5.exp > /dev/null

$(objpfx)tst-array5-static.out: $(objpfx)tst-array5-static
	$(objpfx)tst-array5-static > $@
	cmp $@ tst-array5-static.exp > /dev/null

ifeq (yesyes,$(have-fpie)$(build-shared))
CFLAGS-tst-pie1.c += $(pie-ccflag)

$(objpfx)tst-pie1.out: $(objpfx)tst-pie1
	$(elf-objpfx)$(rtld-installed-name) \
	  --library-path $(rpath-link)$(patsubst %,:%,$(sysdep-library-path)) \
	  $< > $@

$(objpfx)tst-pie1: $(objpfx)tst-pie1.o $(objpfx)tst-piemod1.so
	$(LINK.o) -pie -Wl,-O1 \
	  $(sysdep-LDFLAGS) $(config-LDFLAGS) \
	  $(extra-B-$(@F:lib%.so=%).so) -B$(csu-objpfx) \
	  $(extra-B-$(@F:lib%.so=%).so) $(load-map-file) \
	  $(LDFLAGS) $(LDFLAGS-$(@F)) \
	  -L$(subst :, -L,$(rpath-link)) -Wl,-rpath-link=$(rpath-link) \
	  -o $@ $(objpfx)tst-pie1.o $(objpfx)tst-piemod1.so \
	  $(common-objpfx)libc_nonshared.a

generated += tst-pie1 tst-pie1.out tst-pie1.o
endif

check-textrel-CFLAGS = -O -Wall -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
$(objpfx)check-textrel: check-textrel.c
	$(native-compile)

check-localplt-CFLAGS = -O -Wall -D_GNU_SOURCE -std=gnu99
$(objpfx)check-localplt: check-localplt.c
	$(native-compile)

ifeq (yes,$(build-shared))
tests: $(objpfx)check-textrel.out

$(objpfx)check-textrel.out: $(objpfx)check-textrel
	$(dir $<)$(notdir $<) $(common-objpfx)libc.so \
	  $(sort $(wildcard $(common-objpfx)*/lib*.so \
			    $(common-objpfx)iconvdata/*.so)) > $@
generated += check-textrel check-textrel.out

$(objpfx)tst-dlmodcount: $(libdl)
$(objpfx)tst-dlmodcount.out: $(test-modules)

check-data := $(firstword $(wildcard \
	        $(foreach D,$(add-ons) scripts,\
	        	  $(patsubst %,$(..)$D/data/localplt-%.data,\
			   	     $(abi-name) \
			   	     $(addsuffix -$(config-os),\
				     		 $(config-machine) \
						 $(base-machine)) \
			   generic))))

tests: $(objpfx)check-localplt.out

ifeq ($(have-thread-library),yes)
thread-dso := $(filter-out %_nonshared.a, $(shared-thread-library))
endif

$(objpfx)check-localplt.out: $(objpfx)check-localplt \
			     $(common-objpfx)libc.so \
		    	     $(common-objpfx)math/libm.so $(thread-dso) \
		    	     $(common-objpfx)rt/librt.so \
		    	     $(common-objpfx)dlfcn/libdl.so \
			     $(common-objpfx)resolv/libresolv.so \
			     $(common-objpfx)crypt/libcrypt.so \
			     $(check-data)
	$(dir $<)$(notdir $<) $(filter-out $< $(check-data),$^) | \
	  LC_ALL=C sort | \
	  diff -u $(check-data) - > $@
endif

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

$(objpfx)tst-audit2.out: $(objpfx)tst-auditmod1.so
tst-audit2-ENV = LD_AUDIT=$(objpfx)tst-auditmod1.so

$(objpfx)tst-global1: $(libdl)
$(objpfx)tst-global1.out: $(objpfx)testobj6.so $(objpfx)testobj2.so

$(objpfx)order2: $(libdl)
$(objpfx)order2.out: $(objpfx)order2 $(objpfx)order2mod1.so \
		     $(objpfx)order2mod2.so
	$(elf-objpfx)$(rtld-installed-name) \
	  --library-path $(rpath-link)$(patsubst %,:%,$(sysdep-library-path)) \
	  $(objpfx)order2 > $@
	(echo "12345" | cmp $@ -) > /dev/null
$(objpfx)order2mod1.so: $(objpfx)order2mod4.so
$(objpfx)order2mod4.so: $(objpfx)order2mod3.so
$(objpfx)order2mod2.so: $(objpfx)order2mod3.so
order2mod2.so-no-z-defs = yes

tst-stackguard1-ARGS = --command "$(built-program-cmd) --child"
tst-stackguard1-static-ARGS = --command "$(objpfx)tst-stackguard1-static --child"

$(objpfx)tst-leaks1: $(libdl)
$(objpfx)tst-leaks1-mem: $(objpfx)tst-leaks1.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-leaks1.mtrace > $@

tst-leaks1-ENV = MALLOC_TRACE=$(objpfx)tst-leaks1.mtrace

$(objpfx)tst-addr1: $(libdl)

$(objpfx)tst-thrlock: $(libdl) $(shared-thread-library)
