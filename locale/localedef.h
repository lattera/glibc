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

#ifndef _LOCALEDEF_H
#define _LOCALEDEF_H 1

#define __need_wchar_t
#include <stddef.h>

#include "config.h"

#include "hash.h"


/* Needed always.  */
#define MAX(a, b) ({typeof (a) _a = (a); typeof (b) _b = (b);               \
                    _a > _b ? _a : _b; })
#define MIN(a, b) ({typeof (a) _a = (a); typeof (b) _b = (b);               \
                    _a < _b ? _a : _b; })

/* Determine number of elements in ARR.  */
#define NELEMS(arr) ((sizeof (arr)) / (sizeof (arr[0])))

/* I simply love these GCC features ... :) */
#define NO_PAREN(arg, rest...) arg, ##rest


/* The character set used in the locale is defined in a character map file.
   The information of the file is stored in the following struct.  */
struct charmap
  {
    char *filename;
    char *codeset_name;
    int mb_cur_min;
    int mb_cur_max;
    char escape_char;
    char comment_char;
    hash_table table;
    int hash_size;
    int hash_layers;
  };

/* Data structure for representing charmap database.  */
extern struct charmap charmap_data;


/* We can map the types of the entries into four categories.  */
enum value_type { none, string, stringarray, byte, bytearray, integer };

/* Definition of the data structure which represents a category and its
   items.  */
struct category
{
  int cat_id;
  const char *name;
  size_t number;
  struct cat_item
  {
    int item_id;
    const char *name;
    enum { std, opt } status;
    enum value_type value_type;
    int min;
    int max;
  } *item_desc;
  char **item_value;
  void (*infct)(int);
  void (*checkfct)(void);
  int (*outfct)(void);
  int filled;
  char *copy_locale;
};

/* This a the structure which contains all information about all
   categories.  */
extern struct category category[];


/* The function used to load the contents of a charmap file into the
   the global variable `charmap_data'.  */
void charmap_read (const char *filename);

/* Find a character constant given by its name in the hash table.  */
static inline wchar_t find_char (const char *name, size_t len)
{
  wchar_t retval;
  if (find_entry (&charmap_data.table, name, len, (void **) &retval) != 0)
    return retval;
  else
    return -1;
}

/* Path to the directory the output files are written in.  */
extern char *output_path;

/* If this is defined be POSIX conform.  */
extern int posix_conformance;

/* If not zero give a lot more messages.  */
extern int verbose;

/* This structure contains all informations about the status of of
   reading the locale definition file.  */
struct locfile_data
  {
    const char *filename;
    char escape_char;
    char comment_char;
    size_t bufsize;
    char *buf;
    char *strbuf;
    size_t buf_ptr;
    int continue_line;
    size_t returned_tokens;
    size_t line_no;
  };

/* The status variable.  */
extern struct locfile_data locfile_data;

/* Open the locale definition file.  */
void locfile_open (const char *fname);

/* Return the next token from the locale definition file.  */
int locfile_lex (char **token, int *token_len);
/* Dito, but check for EOF.  */
int xlocfile_lex (char **token, int *token_len);

/* Ignore the rest of the line.  First TOKEN given if != 0.  Warn about
   anything other than end of line if WARN_FLAG nonzero.  */
void ignore_to_eol (int token, int warn_flag);

/* Code a character with UTF-8 if the character map has multi-byte
   characters.  */
int char_to_utf (char *buf, int char_val);


/* Read the locale defintion file FNAME and fill the appropriate
   data structures.  */
void locfile_read (const char *fname);

/* Check categories for consistency.  */
void categories_check (void);

/* Write out the binary representation of the category data.  */
void categories_write (void);


/* Treat reading the LC_COLLATE definition.  */
void collate_input (int token);

/* Treat reading the LC_CTYPE definition.  */
void ctype_input (int token);
void ctype_check (void);
int ctype_output (void);

/* Treat reading the LC_MONETARY definition.  */
void monetary_check (void);

/* Treat reading the LC_MESSAGES definition.  */
void messages_check (void);

/* Treat reading the LC_NUMERIC definition.  */
void numeric_check (void);


/* Print an error message, possibly with NLS.  */
void error (int status, int errnum, const char *format, ...)
     __attribute__ ((format (printf, 3, 4)));

/* Library functions.  */
void *xmalloc (size_t n);
void *xcalloc (size_t n, size_t s);
void *xrealloc (void *p, size_t n);

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
#endif /* _LOCALEDEF_H */
