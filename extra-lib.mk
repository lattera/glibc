# This file is included several times in a row, once
# for each element of $(extra-libs).  $(extra-libs-left)
# is initialized first to $(extra-libs) so that with each
# inclusion, we advance $(lib) to the next library name (e.g. libfoo).
# The variable $($(lib)-routines) defines the list of modules
# to be included in that library.

lib := $(firstword $(extra-libs-left))
extra-libs-left := $(filter-out $(lib),$(extra-libs-left))

object-suffixes-$(lib) := $(filter-out $($(lib)-inhibit-o),$(object-suffixes))

ifneq (,$(object-suffixes-$(lib)))

# Make sure these are simply-expanded variables before we append to them,
# since we want the expressions we append to be expanded right now.
install-lib := $(install-lib)
extra-objs := $(extra-objs)

# Add each flavor of library to the lists of things to build and install.
install-lib += $(foreach o,$(object-suffixes-$(lib)),$(lib:lib%=$(libtype$o)))
extra-objs += $(foreach o,$(object-suffixes-$(lib)),\
			$(patsubst %,%$o,$($(lib)-routines)))
alltypes-$(lib) := $(foreach o,$(object-suffixes-$(lib)),\
			     $(objpfx)$(patsubst %,$(libtype$o),\
			     $(lib:lib%=%)))

ifeq (,$(filter $(lib),$(extra-libs-others)))
lib-noranlib: $(alltypes-$(lib))
ifeq (yes,$(build-shared))
lib-noranlib: $(objpfx)$(lib).so$($(lib).so-version)
endif
else
others: $(alltypes-$(lib))
endif

# The linked shared library is never a dependent of lib-noranlib,
# because linking it will depend on libc.so already being built.
ifneq (,$(filter .os,$(object-suffixes-$(lib))))
others: $(objpfx)$(lib).so$($(lib).so-version)
endif


# Use o-iterator.mk to generate a rule for each flavor of library.
define o-iterator-doit
$(objpfx)$(patsubst %,$(libtype$o),$(lib:lib%=%)): \
  $($(lib)-routines:%=$(objpfx)%$o); $$(build-extra-lib)
endef
object-suffixes-left = $(object-suffixes-$(lib))
include $(patsubst %,$(..)o-iterator.mk,$(object-suffixes-$(lib)))

# Add the version script to the dependencies of the shared library.
$(objpfx)$(lib).so: $(firstword $(wildcard $($(lib)-map) $(lib).map $(..)$(lib).map))

endif
