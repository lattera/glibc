# This uses MIT assembler syntax.  We have no convenient
# way to choose a sysdep file based on MIT vs Motorola syntax.
# No existing m68k ports use Motorola syntax.

crypt := crypt.sun3	# Use uf-crypt/crypt.sun3.S.

# Disgusting magic to get `#'s into the asm code.
$(objpfx)%.o: %.S
	$(CC) $(CPPFLAGS) -E $< \
	| sed 's/(@@@Hash-Here@@@)/#/g' | $(AS) $(ASFLAGS) -o $@
.S-rule := t
