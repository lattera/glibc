# The Sparc`long double' is a distinct type we support.
#long-double-fcts = yes

# But the support for ieee quads is so bad we just skip the whole thing.
# XXX This is only a temporary hack.
omit-long-double-fcts = yes

# But we do need these few functions to even link stdio proggies
ifeq ($(subdir),math)
sysdep_routines += s_isinfl s_isnanl s_finitel s_signbitl
endif
