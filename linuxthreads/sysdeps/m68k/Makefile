ifeq ($(subdir), linuxthreads)
object-suffixes-left := $(libpthread-nonshared)
define o-iterator-doit
$(objpfx)$o.os: pic-ccflag = -fPIC
endef
include $(o-iterator)
endif
