#include <config.h>

#ifdef HAVE_ASM_UNIQUE_OBJECT
# define S(s) _S (s)
# define _S(s) #s

asm (".data;"
     S (ASM_GLOBAL_DIRECTIVE) " var\n"
     ".type var, " S (ASM_TYPE_DIRECTIVE_PREFIX) "gnu_unique_object\n"
     ".size var, 4\n"
     "var:.zero 4\n"
     ".previous");
extern int var;

int
f (int *p)
{
  return &var != p || *p != 1;
}
#endif
