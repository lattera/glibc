ifeq ($(subdir),iconvdata)
ISO-8859-1_CP037_Z900-routines := iso-8859-1_cp037_z900
ISO-8859-1_CP037_Z900-map := gconv.map

UTF8_UTF32_Z9-routines := utf8-utf32-z9
UTF8_UTF32_Z9-map := gconv.map

UTF16_UTF32_Z9-routines := utf16-utf32-z9
UTF16_UTF32_Z9-map := gconv.map

UTF8_UTF16_Z9-routines := utf8-utf16-z9
UTF8_UTF16_Z9-map := gconv.map

s390x-iconv-modules = ISO-8859-1_CP037_Z900 UTF8_UTF16_Z9 UTF16_UTF32_Z9 UTF8_UTF32_Z9

extra-modules-left += $(s390x-iconv-modules)
include extra-module.mk

cpp-srcs-left := $(foreach mod,$(s390x-iconv-modules),$($(mod)-routines))
lib := iconvdata
include $(patsubst %,$(..)libof-iterator.mk,$(cpp-srcs-left))

extra-objs      += $(addsuffix .so, $(s390x-iconv-modules))
install-others  += $(patsubst %, $(inst_gconvdir)/%.so, $(s390x-iconv-modules))

$(patsubst %, $(inst_gconvdir)/%.so, $(s390x-iconv-modules)) : \
$(inst_gconvdir)/%.so: $(objpfx)%.so $(+force)
	$(do-install-program)

sysdeps-gconv-modules = ../sysdeps/s390/gconv-modules
endif

ifeq ($(subdir),elf)
ifeq ($(build-shared),yes)
tests += tst-dl-runtime-resolve-noaudit tst-dl-runtime-resolve-audit \
	tst-dl-runtime-profile-noaudit
# FIXME: When both LD_PROFILE and LD_AUDIT are set, a segfault occurs.
#tests += tst-dl-runtime-profile-audit
modules-names += tst-dl-runtime-mod
$(objpfx)tst-dl-runtime-resolve-noaudit: $(objpfx)tst-dl-runtime-mod.so
$(objpfx)tst-dl-runtime-resolve-audit: $(objpfx)tst-dl-runtime-mod.so
$(objpfx)tst-dl-runtime-profile-noaudit: $(objpfx)tst-dl-runtime-mod.so
$(objpfx)tst-dl-runtime-profile-audit: $(objpfx)tst-dl-runtime-mod.so
$(objpfx)tst-dl-runtime-resolve-audit.out: $(objpfx)tst-auditmod1.so
$(objpfx)tst-dl-runtime-profile-audit.out: $(objpfx)tst-auditmod1.so
# The profiling output goes to $LD_PROFILE_OUTPUT/$LD_PROFILE.profile
env-profile = LD_PROFILE=$(objpfx)tst-dl-runtime-mod.so \
	LD_PROFILE_OUTPUT=/
env-audit = LD_AUDIT=$(objpfx)tst-auditmod1.so
tst-dl-runtime-resolve-audit-ENV = $(env-audit)
tst-dl-runtime-profile-noaudit-ENV = $(env-profile)
tst-dl-runtime-profile-audit-ENV = $(env-profile) $(env-audit)
endif
endif
