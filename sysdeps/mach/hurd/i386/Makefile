ifeq ($(subdir),csu)
extra-objs += crt0.o
install-lib += crt0.o
omit-deps += crt0

$(objpfx)crt0.o: $(objpfx)static-start.o $(objpfx)abi-note.o $(objpfx)init.o
	$(link-relocatable)
endif
