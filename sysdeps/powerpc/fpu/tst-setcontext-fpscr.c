/* Copyright (C) 2001,2002,2004,2006-2008,2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ryan S. Arnold <rsa@us.ibm.com>
                  Sean Curry <spcurry@us.ibm.com>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>
#include <malloc.h>
#include <link.h>
#include <elf.h>
#include <sysdep.h>
#include <fpu_control.h>

static ucontext_t ctx[3];


volatile int global;


static int back_in_main;


volatile static ElfW(auxv_t) *auxv = NULL;

ElfW(Addr) query_auxv(int type)
{
  FILE *auxv_f;
  ElfW(auxv_t) auxv_struct;
  ElfW(auxv_t) *auxv_temp;
  int i = 0;

  /* if the /proc/self/auxv file has not been manually copied into the heap
     yet, then do it */

  if(auxv == NULL)
    {
      auxv_f = fopen("/proc/self/auxv", "r");

      if(auxv_f == 0)
	{
	  perror("Error opening file for reading");
	  return 0;
	}
      auxv = (ElfW(auxv_t) *)malloc(getpagesize());

      do
	{
	  fread(&auxv_struct, sizeof(ElfW(auxv_t)), 1, auxv_f);
	  auxv[i] = auxv_struct;
	  i++;
	} while(auxv_struct.a_type != AT_NULL);
    }

  auxv_temp = (ElfW(auxv_t) *)auxv;
  i = 0;
  do
    {
      if(auxv_temp[i].a_type == type)
	{
	  return auxv_temp[i].a_un.a_val;
	}
      i++;
    } while (auxv_temp[i].a_type != AT_NULL);

  return 0;
}

typedef unsigned long long di_fpscr_t __attribute__ ((__mode__ (__DI__)));
typedef unsigned int si_fpscr_t __attribute__ ((__mode__ (__SI__)));

#define _FPSCR_RESERVED 0xfffffff8ffffff04ULL

#define _FPSCR_TEST0_DRN 0x0000000400000000ULL
#define _FPSCR_TEST0_RN  0x0000000000000003ULL

#define _FPSCR_TEST1_DRN 0x0000000300000000ULL
#define _FPSCR_TEST1_RN  0x0000000000000002ULL

/* Macros for accessing the hardware control word on Power6[x].  */
# define _GET_DI_FPSCR(__fpscr) ({					     \
   union { double d;							     \
           di_fpscr_t fpscr; }						     \
     tmp __attribute__ ((__aligned__(8)));				     \
   __asm__ ("mffs 0; stfd%U0 0,%0" : "=m" (tmp.d) : : "fr0");		     \
   (__fpscr)=tmp.fpscr;							     \
   tmp.fpscr; })

/* We make sure to zero fp0 after we use it in order to prevent stale data
   in an fp register from making a test-case pass erroneously.  */
# define _SET_DI_FPSCR(__fpscr) {					     \
  union { double d; di_fpscr_t fpscr; }					     \
    tmp __attribute__ ((__aligned__(8)));				     \
  tmp.fpscr = __fpscr;							     \
  /* Set the entire 64-bit FPSCR.  */					     \
  __asm__ ("lfd%U0 0,%0; "						     \
	   ".machine push; "						     \
	   ".machine \"power6\"; "					     \
	   "mtfsf 255,0,1,0; "						     \
	   ".machine pop" : : "m" (tmp.d) : "fr0");			     \
  tmp.d = 0;								     \
  __asm__("lfd%U0 0,%0" : : "m" (tmp.d) : "fr0");			     \
}

# define _GET_SI_FPSCR(__fpscr) ({					     \
   union { double d;							     \
           si_fpscr_t cw[2]; }						     \
     tmp __attribute__ ((__aligned__(8)));				     \
   __asm__ ("mffs 0; stfd%U0 0,%0" : "=m" (tmp.d) : : "fr0");		     \
   (__fpscr)=tmp.cw[1];							     \
   tmp.cw[0]; })

/* We make sure to zero fp0 after we use it in order to prevent stale data
   in an fp register from making a test-case pass erroneously.  */
# define _SET_SI_FPSCR(__fpscr) {					     \
  union { double d; si_fpscr_t fpscr[2]; }				     \
    tmp __attribute__ ((__aligned__(8)));				     \
  /* More-or-less arbitrary; this is a QNaN. */				     \
  tmp.fpscr[0] = 0xFFF80000;						     \
  tmp.fpscr[1] = __fpscr;						     \
  __asm__ ("lfd%U0 0,%0; mtfsf 255,0" : : "m" (tmp.d) : "fr0");		     \
  tmp.d = 0;								     \
  __asm__("lfd%U0 0,%0" : : "m" (tmp.d) : "fr0");			     \
}

void prime_special_regs(int which)
{
  ElfW(Addr) a_val;

  di_fpscr_t di_fpscr __attribute__ ((__aligned__(8)));

  a_val = query_auxv(AT_HWCAP);
  if(a_val == -1)
    {
      puts ("querying the auxv for the hwcap failed");
      _exit (1);
    }

  /* Indicates a 64-bit FPSCR.  */
  if (a_val & PPC_FEATURE_HAS_DFP)
    {
      _GET_DI_FPSCR(di_fpscr);

      /* Overwrite the existing DRN and RN if there is one.  */
      if (which == 0)
        di_fpscr = ((di_fpscr & _FPSCR_RESERVED) | (_FPSCR_TEST0_DRN | _FPSCR_TEST0_RN));
      else
        di_fpscr = ((di_fpscr & _FPSCR_RESERVED) | (_FPSCR_TEST1_DRN | _FPSCR_TEST1_RN));
      puts ("Priming 64-bit FPSCR with:");
      printf("0x%.16llx\n",(unsigned long long int)di_fpscr);

      _SET_DI_FPSCR(di_fpscr);
    }
  else
    {
      puts ("32-bit FPSCR found and will be tested.");
      _GET_SI_FPSCR(di_fpscr);

      /* Overwrite the existing RN if there is one.  */
      if (which == 0)
        di_fpscr = ((di_fpscr & _FPSCR_RESERVED) | (_FPSCR_TEST0_RN));
      else
        di_fpscr = ((di_fpscr & _FPSCR_RESERVED) | (_FPSCR_TEST1_RN));
      puts ("Priming 32-bit FPSCR with:");
      printf("0x%.8lx\n",(unsigned long int) di_fpscr);

      _SET_SI_FPSCR(di_fpscr);
    }
}

void clear_special_regs(void)
{
  ElfW(Addr) a_val;

  di_fpscr_t di_fpscr __attribute__ ((__aligned__(8)));

  union {
	  double d;
	  unsigned long long int lli;
	  unsigned int li[2];
  } dlli;

  a_val = query_auxv(AT_HWCAP);
  if(a_val == -1)
    {
      puts ("querying the auxv for the hwcap failed");
      _exit (1);
    }

#if __WORDSIZE == 32
  dlli.d = ctx[0].uc_mcontext.uc_regs->fpregs.fpscr;
#else
  dlli.d = ctx[0].uc_mcontext.fp_regs[32];
#endif

  puts("The FPSCR value saved in the ucontext_t is:");

  /* Indicates a 64-bit FPSCR.  */
  if (a_val & PPC_FEATURE_HAS_DFP)
    {
      printf("0x%.16llx\n",dlli.lli);
      di_fpscr = 0x0;
      puts ("Clearing the 64-bit FPSCR to:");
      printf("0x%.16llx\n",(unsigned long long int) di_fpscr);

      _SET_DI_FPSCR(di_fpscr);
    }
  else
    {
      printf("0x%.8x\n",(unsigned int) dlli.li[1]);
      di_fpscr = 0x0;
      puts ("Clearing the 32-bit FPSCR to:");
      printf("0x%.8lx\n",(unsigned long int) di_fpscr);

      _SET_SI_FPSCR(di_fpscr);
    }
}

void test_special_regs(int which)
{
  ElfW(Addr) a_val;
  unsigned long long int test;

  di_fpscr_t di_fpscr __attribute__ ((__aligned__(8)));

  a_val = query_auxv(AT_HWCAP);
  if(a_val == -1)
    {
      puts ("querying the auxv for the hwcap failed");
      _exit (2);
    }

  /* Indicates a 64-bit FPSCR.  */
  if (a_val & PPC_FEATURE_HAS_DFP)
    {
      _GET_DI_FPSCR(di_fpscr);

      if (which == 0)
	puts ("After setcontext the 64-bit FPSCR contains:");
      else
	puts ("After swapcontext the 64-bit FPSCR contains:");

      printf("0x%.16llx\n",(unsigned long long int) di_fpscr);
      test = (_FPSCR_TEST0_DRN | _FPSCR_TEST0_RN);
      if((di_fpscr & (test)) != (test))
        {
	  printf ("%s: DRN and RN bits set before getcontext were not preserved across [set|swap]context call: %m",__FUNCTION__);
	  _exit (3);
        }
    }
  else
    {
      _GET_SI_FPSCR(di_fpscr);
      if (which == 0)
	puts ("After setcontext the 32-bit FPSCR contains:");
      else
	puts ("After swapcontext the 32-bit FPSCR contains:");

      printf("0x%.8lx\n",(unsigned long int) di_fpscr);
      test = _FPSCR_TEST0_RN;
      if((di_fpscr & test) != test)
        {
	  printf ("%s: RN bit set before getcontext was not preserved across [set|swap]context call: %m",__FUNCTION__);
	  _exit (4);
        }
    }
}


static void
check_called (void)
{
  if (back_in_main == 0)
    {
      puts ("program did not reach main again");
      _exit (5);
    }
}


int
main (void)
{
  atexit (check_called);

  puts ("priming the FPSCR with a marker");
  prime_special_regs (0);

  puts ("making contexts");
  if (getcontext (&ctx[0]) != 0)
    {
      if (errno == ENOSYS)
	{
	  back_in_main = 1;
	  exit (0);
	}

      printf ("%s: getcontext: %m\n", __FUNCTION__);
      exit (6);
    }

  /* Play some tricks with this context.  */
  if (++global == 1)
    {
    clear_special_regs ( );
    if (setcontext (&ctx[0]) != 0)
      {
	printf ("%s: setcontext: %m\n", __FUNCTION__);
	exit (7);
      }
    }
  if (global != 2)
    {
      printf ("%s: 'global' not incremented twice\n", __FUNCTION__);
      exit (8);
    }

  test_special_regs (0);

  global = 0;
  if (getcontext (&ctx[0]) != 0)
    {
      printf ("%s: getcontext: %m\n", __FUNCTION__);
      exit (9);
    }

  if (++global == 1)
    {
      puts ("priming the FPSCR with a marker");
      prime_special_regs (1);

      puts ("swapping contexts");
      if (swapcontext (&ctx[1], &ctx[0]) != 0)
        {
          printf ("%s: swapcontext: %m\n", __FUNCTION__);
          exit (9);
        }
    }
  if (global != 2)
    {
      printf ("%s: 'global' not incremented twice\n", __FUNCTION__);
      exit (10);
    }

  test_special_regs (1);

  puts ("back at main program");
  back_in_main = 1;

  puts ("test succeeded");
  return 0;
}
