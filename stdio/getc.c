#include <ansidecl.h>
#include <stdio.h>
#undef	getc
#define	fgetc	getc
#include <fgetc.c>
