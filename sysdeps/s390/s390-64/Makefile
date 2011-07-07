pic-ccflag = -fpic

ifeq ($(subdir),gmon)
sysdep_routines += s390x-mcount
endif

ifeq ($(subdir),elf)
CFLAGS-rtld.c += -Wno-uninitialized -Wno-unused
CFLAGS-dl-load.c += -Wno-unused
CFLAGS-dl-reloc.c += -Wno-unused
endif

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

extra-objs      += $(addsuffix .so, $(s390x-iconv-modules))
install-others  += $(patsubst %, $(inst_gconvdir)/%.so, $(s390x-iconv-modules))

distribute += iso-8859-1_cp037_z900.c utf8-utf32-z9.c utf16-utf32-z9.c utf8-utf16-z9.c

$(patsubst %, $(inst_gconvdir)/%.so, $(s390x-iconv-modules)) : \
$(inst_gconvdir)/%.so: $(objpfx)%.so $(+force)
	$(do-install-program)

$(objpfx)gconv-modules-s390: gconv-modules $(+force)
	cp $< $@
	echo >> $@
	echo "# S/390 hardware accelerated modules" >> $@
	echo -n "module	ISO-8859-1//		IBM037//	" >> $@
	echo "	ISO-8859-1_CP037_Z900	1" >> $@
	echo -n "module	IBM037//		ISO-8859-1//	" >> $@
	echo "	ISO-8859-1_CP037_Z900	1" >> $@
	echo -n "module	ISO-10646/UTF8/		UTF-32//	" >> $@
	echo "	UTF8_UTF32_Z9		1" >> $@
	echo -n "module	UTF-32BE//		ISO-10646/UTF8/	" >> $@
	echo "	UTF8_UTF32_Z9		1" >> $@
	echo -n "module	ISO-10646/UTF8/		UTF-32BE//	" >> $@
	echo "	UTF8_UTF32_Z9		1" >> $@
	echo -n "module	UTF-16BE//		UTF-32//	" >> $@
	echo "	UTF16_UTF32_Z9		1" >> $@
	echo -n "module	UTF-32BE//		UTF-16//	" >> $@
	echo "	UTF16_UTF32_Z9		1" >> $@
	echo -n "module	INTERNAL		UTF-16//	" >> $@
	echo "	UTF16_UTF32_Z9		1" >> $@
	echo -n "module	UTF-32BE//		UTF-16BE//	" >> $@
	echo "	UTF16_UTF32_Z9		1" >> $@
	echo -n "module	INTERNAL		UTF-16BE//	" >> $@
	echo "	UTF16_UTF32_Z9		1" >> $@
	echo -n "module	UTF-16BE//		UTF-32BE//	" >> $@
	echo "	UTF16_UTF32_Z9		1" >> $@
	echo -n "module	UTF-16BE//		INTERNAL	" >> $@
	echo "	UTF16_UTF32_Z9		1" >> $@
	echo -n "module	UTF-16BE//		ISO-10646/UTF8/	" >> $@
	echo "	UTF8_UTF16_Z9		1" >> $@
	echo -n "module	ISO-10646/UTF8/		UTF-16//	" >> $@
	echo "	UTF8_UTF16_Z9		1" >> $@
	echo -n "module	ISO-10646/UTF8/		UTF-16BE//	" >> $@
	echo "	UTF8_UTF16_Z9		1" >> $@

$(inst_gconvdir)/gconv-modules: $(objpfx)gconv-modules-s390 $(+force)
	$(do-install)
ifeq (no,$(cross-compiling))
# Update the $(prefix)/lib/gconv/gconv-modules.cache file. This is necessary
# if this libc has more gconv modules than the previously installed one.
	if test -f "$(inst_gconvdir)/gconv-modules.cache"; then \
	   LC_ALL=C LANGUAGE=C \
	   $(common-objpfx)elf/ld.so --library-path $(rpath-link) \
	   $(common-objpfx)iconv/iconvconfig \
	     $(addprefix --prefix=,$(install_root)); \
	fi
else
	@echo '*@*@*@ You should recreate $(inst_gconvdir)/gconv-modules.cache'
endif

endif
