/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <endian.h>
#include "localeinfo.h"

/* These tables' entries contain values which make the function behave
   according to POSIX.2 Table 2-8 ``LC_COLLATE Category Definition in
   the POSIX Locale''.  */

const u_int32_t _nl_C_LC_COLLATE_symbol_hash[446] =
{
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0x00000154u, 0x00000060u, 0xffffffffu, 0xffffffffu,
  0x0000004fu, 0x0000001au, 0x00000085u, 0x00000030u, 0xffffffffu, 0xffffffffu,
  0x000002beu, 0x000000fau, 0xffffffffu, 0xffffffffu, 0x0000014eu, 0x0000005eu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x000000bbu, 0x00000044u,
  0xffffffffu, 0xffffffffu, 0x000000efu, 0x0000004cu, 0x00000147u, 0x0000005cu,
  0x000000a0u, 0x0000003eu, 0x00000000u, 0x00000000u, 0x00000038u, 0x00000016u,
  0x00000094u, 0x00000038u, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0x00000140u, 0x0000005au, 0x0000018cu, 0x00000076u,
  0x0000007du, 0x0000002cu, 0xffffffffu, 0xffffffffu, 0x00000115u, 0x00000052u,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x00000285u, 0x000000deu,
  0x00000171u, 0x0000006cu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0x00000289u, 0x000000e2u, 0x000002d8u, 0x000000feu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x00000022u, 0x00000010u,
  0x0000028fu, 0x000000e8u, 0x00000069u, 0x00000022u, 0x0000006du, 0x00000024u,
  0x00000071u, 0x00000026u, 0x00000075u, 0x00000028u, 0xffffffffu, 0xffffffffu,
  0x00000295u, 0x000000eeu, 0xffffffffu, 0xffffffffu, 0x00000297u, 0x000000f0u,
  0xffffffffu, 0xffffffffu, 0x00000299u, 0x000000f2u, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x00000213u, 0x000000b6u,
  0xffffffffu, 0xffffffffu, 0x00000014u, 0x0000000au, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0x00000227u, 0x000000b8u, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x0000015du, 0x00000064u,
  0xffffffffu, 0xffffffffu, 0x000001ffu, 0x000000a2u, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x0000013au, 0x00000058u,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x00000010u, 0x00000008u,
  0x000001dfu, 0x00000082u, 0x000001e1u, 0x00000084u, 0x00000167u, 0x00000068u,
  0x00000004u, 0x00000002u, 0x000001e7u, 0x0000008au, 0x00000186u, 0x00000074u,
  0x000001ebu, 0x0000008eu, 0x000001edu, 0x00000090u, 0x000001efu, 0x00000092u,
  0x000001f1u, 0x00000094u, 0x000001f3u, 0x00000096u, 0x000001f5u, 0x00000098u,
  0x000001f7u, 0x0000009au, 0x000001f9u, 0x0000009cu, 0x000001a5u, 0x0000007au,
  0x000001fdu, 0x000000a0u, 0x00000030u, 0x00000014u, 0x00000201u, 0x000000a4u,
  0x00000203u, 0x000000a6u, 0x00000205u, 0x000000a8u, 0x00000207u, 0x000000aau,
  0x00000209u, 0x000000acu, 0x0000020bu, 0x000000aeu, 0x0000020du, 0x000000b0u,
  0x0000020fu, 0x000000b2u, 0x00000211u, 0x000000b4u, 0xffffffffu, 0xffffffffu,
  0x0000009cu, 0x0000003cu, 0xffffffffu, 0xffffffffu, 0x00000098u, 0x0000003au,
  0x0000016cu, 0x0000006au, 0xffffffffu, 0xffffffffu, 0x00000269u, 0x000000c2u,
  0x0000026bu, 0x000000c4u, 0x0000026du, 0x000000c6u, 0x0000026fu, 0x000000c8u,
  0x00000271u, 0x000000cau, 0x00000273u, 0x000000ccu, 0x00000275u, 0x000000ceu,
  0x00000277u, 0x000000d0u, 0x00000279u, 0x000000d2u, 0x0000027bu, 0x000000d4u,
  0x0000027du, 0x000000d6u, 0x0000027fu, 0x000000d8u, 0x00000281u, 0x000000dau,
  0x00000283u, 0x000000dcu, 0x00000090u, 0x00000036u, 0x00000287u, 0x000000e0u,
  0x0000005fu, 0x0000001cu, 0x0000028bu, 0x000000e4u, 0x0000028du, 0x000000e6u,
  0x00000089u, 0x00000032u, 0x000001c3u, 0x0000007eu, 0x00000293u, 0x000000ecu,
  0x00000062u, 0x0000001eu, 0x000001b1u, 0x0000007cu, 0x00000130u, 0x00000056u,
  0x0000029bu, 0x000000f4u, 0x00000196u, 0x00000078u, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0x00000081u, 0x0000002eu, 0x00000251u, 0x000000beu,
  0x00000079u, 0x0000002au, 0x0000029du, 0x000000f6u, 0xffffffffu, 0xffffffffu,
  0x0000025cu, 0x000000c0u, 0xffffffffu, 0xffffffffu, 0x0000002cu, 0x00000012u,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x000000a4u, 0x00000040u,
  0xffffffffu, 0xffffffffu, 0x000002b0u, 0x000000f8u, 0xffffffffu, 0xffffffffu,
  0x000000f9u, 0x0000004eu, 0xffffffffu, 0xffffffffu, 0x0000001cu, 0x0000000eu,
  0xffffffffu, 0xffffffffu, 0x0000017bu, 0x00000070u, 0x0000000cu, 0x00000006u,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x000001e3u, 0x00000086u,
  0xffffffffu, 0xffffffffu, 0x000001e5u, 0x00000088u, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0x000001d1u, 0x00000080u, 0x000001e9u, 0x0000008cu,
  0x0000008cu, 0x00000034u, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0x00000291u, 0x000000eau, 0xffffffffu, 0xffffffffu,
  0x00000008u, 0x00000004u, 0xffffffffu, 0xffffffffu, 0x00000181u, 0x00000072u,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x00000231u, 0x000000bau,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x000000cau, 0x00000046u,
  0x00000246u, 0x000000bcu, 0xffffffffu, 0xffffffffu, 0x000001fbu, 0x0000009eu,
  0x000000d6u, 0x00000048u, 0x00000018u, 0x0000000cu, 0xffffffffu, 0xffffffffu,
  0x00000159u, 0x00000062u, 0xffffffffu, 0xffffffffu, 0x000000aau, 0x00000042u,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x000000e2u, 0x0000004au,
  0x00000175u, 0x0000006eu, 0xffffffffu, 0xffffffffu, 0x00000104u, 0x00000050u,
  0x00000065u, 0x00000020u, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x000002d2u, 0x000000fcu,
  0xffffffffu, 0xffffffffu, 0x00000161u, 0x00000066u, 0x00000045u, 0x00000018u,
  0xffffffffu, 0xffffffffu, 0x00000127u, 0x00000054u, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
  0xffffffffu, 0xffffffffu
};

const char _nl_C_LC_COLLATE_symbol_strings[732] =
  "NUL\0" "SOH\0" "STX\0" "ETX\0" "EOT\0" "ENQ\0" "ACK\0" "alert\0"
  "backspace\0" "tab\0" "newline\0" "vertical-tab\0" "form-feed\0"
  "carriage-return\0" "SI\0" "SO\0" "DLE\0" "DC1\0" "DC2\0" "DC3\0" "DC4\0"
  "NAK\0" "SYN\0" "ETB\0" "CAN\0" "EM\0" "SUB\0" "ESC\0" "IS4\0" "IS3\0"
  "IS2\0" "IS1\0" "space\0" "exclamation-mark\0" "quotation-mark\0"
  "number-sign\0" "dollar-sign\0" "percent-sign\0" "ampersand\0"
  "apostrophe\0" "left-parenthesis\0" "right-parenthesis\0" "asterisk\0"
  "plus-sign\0" "comma\0" "hyphen\0" "period\0" "slash\0" "zero\0" "one\0"
  "two\0" "three\0" "four\0" "five\0" "six\0" "seven\0" "eight\0" "nine\0"
  "colon\0" "semicolon\0" "less-than-sign\0" "equals-sign\0"
  "greater-than-sign\0" "question-mark\0" "commercial-at\0" "A\0" "B\0" "C\0"
  "D\0" "E\0" "F\0" "G\0" "H\0" "I\0" "J\0" "K\0" "L\0" "M\0" "N\0" "O\0"
  "P\0" "Q\0" "R\0" "S\0" "T\0" "U\0" "V\0" "W\0" "X\0" "Y\0" "Z\0"
  "left-square-bracket\0" "backslash\0" "right-square-bracket\0"
  "circumflex\0" "underscore\0" "grave-accent\0" "a\0" "b\0" "c\0" "d\0" "e\0"
  "f\0" "g\0" "h\0" "i\0" "j\0" "k\0" "l\0" "m\0" "n\0" "o\0" "p\0" "q\0"
  "r\0" "s\0" "t\0" "u\0" "v\0" "w\0" "x\0" "y\0" "z\0" "left-curly-bracket\0"
  "vertical-line\0" "right-curly-bracket\0" "tilde\0" "DEL\0";

const u_int32_t _nl_C_LC_COLLATE_symbol_classes[256] =
{
  1,   0, 1,   1, 1,   2, 1,   3, 1,   4, 1,   5, 1,   6, 1,   7,
  1,   8, 1,   9, 1,  10, 1,  11, 1,  12, 1,  13, 1,  14, 1,  15,
  1,  16, 1,  17, 1,  18, 1,  19, 1,  20, 1,  21, 1,  22, 1,  23,
  1,  24, 1,  25, 1,  26, 1,  27, 1,  28, 1,  29, 1,  30, 1,  31,
  1,  32, 1,  33, 1,  34, 1,  35, 1,  36, 1,  37, 1,  38, 1,  39,
  1,  40, 1,  41, 1,  42, 1,  43, 1,  44, 1,  45, 1,  46, 1,  47,
  1,  48, 1,  49, 1,  50, 1,  51, 1,  52, 1,  53, 1,  54, 1,  55,
  1,  56, 1,  57, 1,  58, 1,  59, 1,  60, 1,  61, 1,  62, 1,  63,
  1,  64, 1,  65, 1,  66, 1,  67, 1,  68, 1,  69, 1,  70, 1,  71,
  1,  72, 1,  73, 1,  74, 1,  75, 1,  76, 1,  77, 1,  78, 1,  79,
  1,  80, 1,  81, 1,  82, 1,  83, 1,  84, 1,  85, 1,  86, 1,  87,
  1,  88, 1,  89, 1,  90, 1,  91, 1,  92, 1,  93, 1,  94, 1,  95,
  1,  96, 1,  97, 1,  98, 1,  99, 1, 100, 1, 101, 1, 102, 1, 103,
  1, 104, 1, 105, 1, 106, 1, 107, 1, 108, 1, 109, 1, 110, 1, 111,
  1, 112, 1, 113, 1, 114, 1, 115, 1, 116, 1, 117, 1, 118, 1, 119,
  1, 120, 1, 121, 1, 122, 1, 123, 1, 124, 1, 125, 1, 126, 1, 127
};

const struct locale_data _nl_C_LC_COLLATE =
{
  _nl_C_name,
  NULL, 0, 0, /* no file mapped */
  UNDELETABLE,
  21,
  {
    { word: 0 },
    { string: NULL },
    { word: 0 },
    { word: 0 },
    { string: NULL },
    { string: NULL },
    { word: 0 },
    { string: NULL },
    { string: NULL },
    { word: 0 },
    { string: NULL },
    { string: NULL },
    { string: NULL },
    { string: NULL },
    { string: NULL },
    { word: 223 },
#if __BYTE_ORDER == __LITTLE_ENDIAN
    { string: NULL },
#endif
    { string: (const char *) _nl_C_LC_COLLATE_symbol_hash },
#if __BYTE_ORDER == __BIG_ENDIAN
    { string: NULL },
#endif
    { string: _nl_C_LC_COLLATE_symbol_strings },
#if __BYTE_ORDER == __LITTLE_ENDIAN
    { string: NULL },
#endif
    { string: (const char *) _nl_C_LC_COLLATE_symbol_classes },
#if __BYTE_ORDER == __BIG_ENDIAN
    { string: NULL },
#endif
  }
};
