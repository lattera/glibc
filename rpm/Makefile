# A start at automated rpm-making for GNU libc.
# Generates an rpm spec file in `CPU-VENDOR-OS' from the list of installed
# files and the `template' file.

all:

subdir := rpm
include ../Makeconfig
include $(common-objpfx)soversions.mk
include $(common-objpfx)version.mk

distinfo := $(wildcard $(subdirs:%=../%/distinfo))
-include $(distinfo)

config = $(config-machine)-$(config-vendor)-$(config-os)

headers := $(foreach d,$(subdirs),$($d-headers))
install-lib := $(foreach d,$(subdirs),$($d-install-lib))
install-lib.so := $(foreach d,$(subdirs),$(filter-out $($d-versioned),\
						      $($d-install-lib.so)))
versioned := $(foreach d,$(subdirs),$($d-versioned))
install-bin := $(foreach d,$(subdirs),$($d-install-bin))
install-sbin := $(foreach d,$(subdirs),$($d-install-sbin))
install-data := $(foreach d,$(subdirs),$($d-install-data))
install-others := $(foreach d,$(subdirs),$($d-install-others))

# Notice things to be installed in /etc.  They get specially marked as
# possibly user-modified config files.
install-sysconf := $(filter $(sysconfdir)/%,$(install-others))
install-others := $(filter-out $(sysconfdir)/%,$(install-others))

# Add the unversioned lib*.so's to install-lib.
install-lib += $(install-lib.so)

# For each versioned lib*.so, add three files:
#   lib*-VERSION.so, lib*.so.MAJOR, and lib*.so
install-lib += $(foreach lib,$(versioned),\
			 $(patsubst %.so,%-$(version).so,$(lib)) \
			 $(lib)$($(lib)-version) $(lib))

# Add libc.a and libc_p.a.
install-lib += $(foreach o,$(filter-out .so,$(object-suffixes)),\
			 $(patsubst %,$(libtype$o),c))

ifeq (yes,$(build-shared))
# Add libc.so itself, which is a special case in Makerules.
ifndef libc.so-version
install-others += $(slibdir)/libc.so
else
install-others += $(libdir)/libc.so \
		  $(slibdir)/libc-$(version).so \
		  $(slibdir)/libc.so$(libc.so-version)
endif
endif


all: glibc-$(version).$(config).rpm

instvars = include lib bin sbin data others

# Generate the rpm spec file for this configuration.
$(config): template $(distinfo) Makefile
	rm -f $@.new
	(sed -e 's%@VERSION@%$(version)%g' $<; \
	 ($(foreach var,$(instvars),\
	  for f in $(install-$(var)); do \
	    echo $(filter-out /,$($(var)dir)/)$$f; \
	  done;\
	 )) | sort; \
	 (for f in $(install-sysconf); do \
	    echo %config $$f; \
	  done) | sort) > $@.new
	mv -f $@.new $@

install-include = $(headers)

instfiles := $(sort $(foreach x,$(instvars),\
			      $(addprefix $(filter-out /,$($xdir)/),\
					  $(install-$x))))

# Build the binary rpm for this configuration.
glibc-$(version).$(config).rpm: $(config) rpmrc $(instfiles)
# rpm is really lame.  You cannot tell it the name of the output file,
# and you can only guess at what the name will turn out to be!
	arch=`uname -m | sed -e 's/i.86/i386/' -e 's/alpha/axp/'`; \
	rm -f $$arch; ln -s . $$arch; \
	rpm --rcfile rpmrc -bb $<; \
	rm -f $$arch; \
	mv -f glibc-$(version)-1.$${arch}.rpm $@

# Avoid time-consuming implicit rule search.
$(instfiles) : ;

.PHONY: clean
clean:
	rm -f $(config) glibc-$(version).$(config).rpm
