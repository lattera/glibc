#include <stdio.h>
#undef	getc
#define	fgetc	getc
#include <fgetc.c>

weak_alias (getc, getc_unlocked)
