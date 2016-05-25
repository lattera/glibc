ifeq ($(subdir),string)
sysdep_routines += strlen strlen-vx strlen-c \
		   strnlen strnlen-vx strnlen-c \
		   strcpy strcpy-vx \
		   stpcpy stpcpy-vx stpcpy-c \
		   strncpy strncpy-vx \
		   stpncpy stpncpy-vx stpncpy-c \
		   strcat strcat-vx strcat-c \
		   strncat strncat-vx strncat-c \
		   strcmp strcmp-vx \
		   strncmp strncmp-vx strncmp-c \
		   strchr strchr-vx strchr-c \
		   strchrnul strchrnul-vx strchrnul-c \
		   strrchr strrchr-vx strrchr-c \
		   strspn strspn-vx strspn-c \
		   strpbrk strpbrk-vx strpbrk-c \
		   strcspn strcspn-vx strcspn-c \
		   memchr memchr-vx \
		   rawmemchr rawmemchr-vx rawmemchr-c \
		   memccpy memccpy-vx memccpy-c \
		   memrchr memrchr-vx memrchr-c \
		   mempcpy
endif

ifeq ($(subdir),wcsmbs)
sysdep_routines += wcslen wcslen-vx wcslen-c \
		   wcsnlen wcsnlen-vx wcsnlen-c \
		   wcscpy wcscpy-vx wcscpy-c \
		   wcpcpy wcpcpy-vx wcpcpy-c \
		   wcsncpy wcsncpy-vx wcsncpy-c \
		   wcpncpy wcpncpy-vx wcpncpy-c \
		   wcscat wcscat-vx wcscat-c \
		   wcsncat wcsncat-vx wcsncat-c \
		   wcscmp wcscmp-vx wcscmp-c \
		   wcsncmp wcsncmp-vx wcsncmp-c \
		   wcschr wcschr-vx wcschr-c \
		   wcschrnul wcschrnul-vx wcschrnul-c \
		   wcsrchr wcsrchr-vx wcsrchr-c \
		   wcsspn wcsspn-vx wcsspn-c \
		   wcspbrk wcspbrk-vx wcspbrk-c \
		   wcscspn wcscspn-vx wcscspn-c \
		   wmemchr wmemchr-vx wmemchr-c \
		   wmemset wmemset-vx wmemset-c \
		   wmemcmp wmemcmp-vx wmemcmp-c
endif

ifeq ($(subdir),iconvdata)
override define generate-8bit-table
$(make-target-directory)
LC_ALL=C $(SHELL) ./gen-8bit.sh $< > $(@:stmp=T)
LC_ALL=C $(SHELL) ../sysdeps/s390/multiarch/gen-8bit.sh $< >> $(@:stmp=T)
$(move-if-change) $(@:stmp=T) $(@:stmp=h)
touch $@
endef
endif

ifeq ($(subdir),iconv)
sysdep_routines += gconv_simple
endif
