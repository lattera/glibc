ifeq ($(subdir),csu)
gen-as-const-headers += tcb-offsets.sym
endif

ifeq ($(subdir),nptl)
CPPFLAGS-pt-crti.S += -fPIC
CPPFLAGS-crtn.S += -fPIC
endif
