# This uses MIT assembler syntax.  We have no convenient
# way to choose a sysdep file based on MIT vs Motorola syntax.
# No existing m68k ports use Motorola syntax.

crypt := crypt.sun3	# Use crypt/crypt.sun3.S.

# Disgusting magic to get `#'s into the asm code.
define compile-command.S
$(CC) $(CPPFLAGS) -E $< \
| sed 's/(@@@Hash-Here@@@)/#/g' | $(AS) $(ASFLAGS) -o $@
endef
