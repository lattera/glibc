# This file is included several times in a row, once
# for each element of $(modules-names).  $(extra-modules-left)
# is initialized first to $(modules-names) so that with each
# inclusion, we advance $(module) to the next name.

module := $(firstword $(extra-modules-left))
extra-modules-left := $(filter-out $(module),$(extra-modules-left))

libof-$(notdir $(module)) := extramodules
