#ifndef LONG_DOUBLE_DENORM_BIAS
# define LONG_DOUBLE_DENORM_BIAS (IEEE854_LONG_DOUBLE_BIAS - 1)
#endif

#define PRINT_FPHEX_LONG_DOUBLE							\
do {										\
      /* The "strange" 80 bit format on ix86 and m68k has an explicit		\
	 leading digit in the 64 bit mantissa.  */				\
      unsigned long long int num;						\
										\
      assert (sizeof (long double) == 12);					\
										\
      num = (((unsigned long long int) fpnum.ldbl.ieee.mantissa0) << 32		\
	     | fpnum.ldbl.ieee.mantissa1);					\
										\
      zero_mantissa = num == 0;							\
										\
      if (sizeof (unsigned long int) > 6)					\
	numstr = _itoa_word (num, numbuf + sizeof numbuf, 16,			\
			     info->spec == 'A');				\
      else									\
	numstr = _itoa (num, numbuf + sizeof numbuf, 16, info->spec == 'A');	\
										\
      /* Fill with zeroes.  */							\
      while (numstr > numbuf + (sizeof numbuf - 64 / 4))			\
	*--numstr = '0';							\
										\
      /* We use a full nibble for the leading digit.  */			\
      leading = *numstr++;							\
										\
      /* We have 3 bits from the mantissa in the leading nibble.		\
	 Therefore we are here using `IEEE854_LONG_DOUBLE_BIAS + 3'.  */	\
      exponent = fpnum.ldbl.ieee.exponent;					\
										\
      if (exponent == 0)							\
	{									\
	  if (zero_mantissa)							\
	    expnegative = 0;							\
	  else									\
	    {									\
	      /* This is a denormalized number.  */				\
	      expnegative = 1;							\
	      /* This is a hook for the m68k long double format, where the	\
		 exponent bias is the same for normalized and denormalized	\
		 numbers.  */							\
	      exponent = LONG_DOUBLE_DENORM_BIAS + 3;				\
	    }									\
	}									\
      else if (exponent >= IEEE854_LONG_DOUBLE_BIAS + 3)			\
	{									\
	  expnegative = 0;							\
	  exponent -= IEEE854_LONG_DOUBLE_BIAS + 3;				\
	}									\
      else									\
	{									\
	  expnegative = 1;							\
	  exponent = -(exponent - (IEEE854_LONG_DOUBLE_BIAS + 3));		\
	}									\
} while (0)

#include <sysdeps/generic/printf_fphex.c>
