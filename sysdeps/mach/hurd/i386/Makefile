ifeq ($(subdir),csu)
extra-objs += crt0.o
install-lib += crt0.o

$(objpfx)crt0.o: $(objpfx)static-start.o $(objpfx)abi-note.o $(objpfx)init.o
	$(link-relocatable)
endif
