# This file is included several times in a row, once
# for each element of $(extra-libs).  $(extra-libs-left)
# is initialized first to $(extra-libs) so that with each
# inclusion, we advance $(lib) to the next library name (e.g. libfoo).
# The variable $($(lib)-routines) defines the list of modules
# to be included in that library.

lib := $(firstword $(extra-libs-left))
extra-libs-left := $(filter-out $(lib),$(extra-libs-left))

object-suffixes-$(lib) := $(filter-out $($(lib)-inhibit-o),$(object-suffixes))

# Make sure these are simply-expanded variables before we append to them,
# since we want the expressions we append to be expanded right now.
install-lib := $(install-lib)
extra-objs := $(extra-objs)

# Add each flavor of library to the lists of things to build and install.
install-lib += $(foreach o,$(object-suffixes-$(lib)),$(lib:lib%=$(libtype$o)))
extra-objs += $(foreach o,$(object-suffixes-$(lib)),$($(lib)-routines:=$o))
alltypes-$(lib) := $(foreach o,$(object-suffixes-$(lib)),\
			     $(objpfx)$(patsubst %,$(libtype$o),\
			     $(lib:lib%=%)))
ifneq (,$(filter .so,$(object-suffixes-$(lib))))
alltypes-$(lib) += $(objpfx)$(lib).so
endif

ifeq (,$($(lib)-no-lib-dep))
lib-noranlib: $(alltypes-$(lib))
else
others: $(alltypes-$(lib))
endif

# Use o-iterator.mk to generate a rule for each flavor of library.
define o-iterator-doit
$(objpfx)$(patsubst %,$(libtype$o),$(lib:lib%=%)): \
  $($(lib)-routines:%=$(objpfx)%$o); $$(build-extra-lib)
endef
object-suffixes-left = $(object-suffixes-$(lib))
include $(patsubst %,$(..)o-iterator.mk,$(object-suffixes-$(lib)))
