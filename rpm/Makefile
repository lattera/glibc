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

# Add libc.so itself, which is a special case in Makerules.
install-others += $(slibdir)/libc.so
ifdef libc.so-version
install-others += $(slibdir)/libc-$(version).so \
		  $(slibdir)/libc.so$(libc.so-version)
endif


glibc-$(version).$(config).rpm: $(config)

all: $(config)

$(config): template $(distinfo) Makefile
	rm -f $@.new
	(sed -e 's%@VERSION@%$(version)%g' $<; \
	 ($(foreach var,include lib bin sbin data others,\
	  for f in $(install-$(var)); do \
	    echo $(filter-out /,$($(var)dir)/)$$f; \
	  done;\
	 )) | sort; \
	 (for f in $(install-sysconf); do \
	    echo %config $$f; \
	  done) | sort) > $@.new
	mv -f $@.new $@

install-include = $(headers)

