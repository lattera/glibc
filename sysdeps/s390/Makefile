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
