ifeq (posix,$(subdir))
sysdep_routines := $(sysdep_routines) pgrpsys sco_getgrp
endif

ifeq (csu,$(subdir))

# SCO uses crt1.o, and expects that single initializer file to also start
# the .init and .fini sections as crti.o normally does.
start-installed-name = crt1.o
start-installed-name-rule = yes

# Link together start.o and crti.o into the expected crt1.o.
# Now crt1.o as initializer and crtn.o as finalizer will work.
$(objpfx)crt1.o: $(objpfx)start.o $(objpfx)crti.o
	$(CC) -nostdlib -nostartfiles -Wl,-r -o $@ $^

endif
