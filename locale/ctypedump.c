/* Copyright (C) 1995 Free Software Foundation, Inc.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ctype.h>
#include <endian.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>        /* Just for htons() */

#include "localedef.h"
#include "localeinfo.h"


/* FIXME: these values should be part of the LC_CTYPE information.  */
#define mb_cur_max 1
#define mb_cur_min 1


#define SWAP32(v)							     \
	((u32) (((((u32) (v)) & 0x000000ff) << 24)			     \
		| ((((u32) (v)) & 0x0000ff00) << 8)			     \
		| ((((u32) (v)) & 0x00ff0000) >> 8)			     \
		| ((((u32) (v)) & 0xff000000) >> 24)))



static inline void
print_short_in_char (unsigned short val)
{
  const unsigned char *p = (const unsigned char *) &val;
  printf ("\"\\%03o\\%03o\"", p[0], p[1]);
}


static inline void
print_int_in_char (unsigned int val)
{
  const unsigned char *p = (const unsigned char *) &val;
  printf ("\"\\%03o\\%03o\\%03o\\%03o\"", p[0], p[1], p[2], p[3]);
}

 
int
ctype_output (void)
{
  int ch;
  int result = 0;
  const char *locname = (getenv ("LC_ALL") ?: getenv ("LC_CTYPE") ?: 
			 getenv ("LANG") ?: "POSIX");

  puts ("#include <endian.h>\n");

  if (mb_cur_max == 1)
    {
      printf ("const char _nl_%s_LC_CTYPE_class[] = \n", locname);
      for (ch = -128; ch < (1 << (8 * MB_CUR_MAX)); ++ch)
        {
	  if (((ch + 128) % 6) == 0)
	    printf ("  /* 0x%02x */ ", ch < 0 ? 256 + ch : ch);
	  print_short_in_char (htons (__ctype_b [ch < 0 ? 256 + ch : ch]));
	  fputc (((ch + 128) % 6) == 5 ? '\n' : ' ', stdout);
        }
      puts (";");
    }

  printf ("#if BYTE_ORDER == %s\n",
	  BYTE_ORDER == LITTLE_ENDIAN ? "LITTLE_ENDIAN" : "BIG_ENDIAN");

  if (mb_cur_max == 1)
    {
      printf ("const char _nl_%s_LC_CTYPE_toupper[] = \n", locname);
      for (ch = -128; ch < (1 << (8 * MB_CUR_MAX)); ++ch)
        {
	  if (((ch + 128) % 3) == 0)
	    printf ("  /* 0x%02x */ ", ch < 0 ? 256 + ch : ch);
	  print_int_in_char (__ctype_toupper[ch < 0 ? 256 + ch : ch]);
	  fputc (((ch + 128) % 3) == 2 ? '\n' : ' ', stdout);
        }
      puts (";");

      printf ("const char _nl_%s_LC_CTYPE_tolower[] = \n", locname);
      for (ch = -128; ch < (1 << (8 * MB_CUR_MAX)); ++ch)
        {
	  if (((ch + 128) % 3) == 0)
	    printf ("  /* 0x%02x */ ", ch < 0 ? 256 + ch : ch);
	  print_int_in_char (__ctype_tolower[ch < 0 ? 256 + ch : ch]);
	  fputc (((ch + 128) % 3) == 2 ? '\n' : ' ', stdout);
        }
      puts (";");
    }
  else
    /* not implemented */;

  printf ("#elif BYTE_ORDER == %s\n",
          BYTE_ORDER == LITTLE_ENDIAN ? "BIG_ENDIAN" : "LITTLE_ENDIAN");

  if (mb_cur_max == 1)
    {
      printf ("const char _nl_%s_LC_CTYPE_toupper[] = \n", locname);
      for (ch = -128; ch < (1 << (8 * MB_CUR_MAX)); ++ch)
        {
	  if (((ch + 128) % 3) == 0)
	    printf ("  /* 0x%02x */ ", ch < 0 ? 256 + ch : ch);
	  print_int_in_char (SWAP32 (__ctype_toupper[ch < 0 ? 256 + ch : ch]));
	  fputc (((ch + 128) % 3) == 2 ? '\n' : ' ', stdout);
        }
      puts (";");

      printf ("const char _nl_%s_LC_CTYPE_tolower[] = \n", locname);
      for (ch = -128; ch < (1 << (8 * MB_CUR_MAX)); ++ch)
        {
	  if (((ch + 128) % 3) == 0)
	    printf ("  /* 0x%02x */ ", ch < 0 ? 256 + ch : ch);
	  print_int_in_char (SWAP32 (__ctype_tolower[ch < 0 ? 256 + ch : ch]));
	  fputc (((ch + 128) % 3) == 2 ? '\n' : ' ', stdout);
        }
      puts (";");
    }
  else
    /* not implemented */;

  puts ("#else\n#error \"BYTE_ORDER\" BYTE_ORDER \" not handled.\"\n#endif\n");

  printf("const struct locale_data _nl_%s_LC_CTYPE = \n\
{\n\
  NULL, 0, /* no file mapped */\n\
  5,\n\
  {\n\
    _nl_C_LC_CTYPE_class,\n\
#ifdef BYTE_ORDER == LITTLE_ENDIAN\n\
    NULL, NULL,\n\
#endif\n\
    _nl_C_LC_CTYPE_toupper,\n\
    _nl_C_LC_CTYPE_tolower,\n\
#ifdef BYTE_ORDER == BIG_ENDIAN\n\
    NULL, NULL,\n\
#endif\n\
  }\n\
};\n", locname);

  return result;
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
