ifeq ($(subdir),csu)

extra-objs += static-start.o

# We need special startup code for statically linked binaries.
$(objpfx)crt0.o: $(objpfx)static-start.o $(objpfx)abi-note.o $(objpfx)init.o
	$(link-relocatable)

endif
