#include <ansidecl.h>
#include <stdio.h>
#undef	putc
#define	fputc	putc
#include <fputc.c>
