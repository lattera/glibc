# This sed script massages native_client/src/untrusted/irt/irt.h into
# the nacl-irt.h used to build libc, by rewriting foo_t and struct bar
# to nacl_abi_foo_t and nacl_abi_bar_t (and eliding forward declarations).
# It doesn't perturb any struct CamelCaps cases, since such names will
# be used only in NaCl-specific interfaces.
/^struct \([a-z][a-z]*\);$/d
/^#include "irt\.h"$/d
/(/!b
s/\([a-z0-9_][a-z0-9_]*\)_t\>/nacl_abi_\1_t/g
s/struct \([a-z0-9_][a-z0-9_]*\)/nacl_abi_\1_t/g
s/nacl_abi_\(u*int[3264ptr]*_t\)/\1/g
s/nacl_abi_\(nacl_irt_\)/\1/g
