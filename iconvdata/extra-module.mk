mod := $(firstword $(extra-modules-left))
extra-modules-left := $(strip $(filter-out $(mod),$(extra-modules-left)))

extra-objs := $(extra-objs) $(patsubst %,%.os,$($(mod)-routines))

$(objpfx)$(mod).so: $(addprefix $(objpfx),$(addsuffix .os,$($(mod)-routines)))\
		    $(common-objpfx)shlib.lds
	$(build-module)

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)$(mod).so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a

ifneq (,$(extra-modules-left))
include extra-module.mk
endif
