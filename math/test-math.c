#include <ansidecl.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void print_trig_stuff __P ((void));

int
DEFUN_VOID(main)
{
  CONST char str[] = "123.456";
  double x,h,li,lr,a,lrr;

  x = atof (str);

  printf ("%g %g\n", x, pow (10.0, 3.0));

  x = sinh(2.0);

  printf("sinh(2.0) = %g\n", x);

  x = sinh(3.0);

  printf("sinh(3.0) = %g\n", x);

  h = hypot(2.0,3.0);

  printf("h=%g\n", h);

  a = atan2(3.0, 2.0);

  printf("atan2(3,2) = %g\n", a);

  lr = pow(h,4.0);

  printf("pow(%g,4.0) = %g\n", h, lr);

  lrr = lr;

  li = 4.0 * a;

  lr = lr / exp(a*5.0);

  printf("%g / exp(%g * 5) = %g\n", lrr, a, lr);

  lrr = li;

  li += 5.0 * log(h);

  printf("%g + 5*log(%g) = %g\n", lrr, h, li);

  printf("cos(%g) = %g,  sin(%g) = %g\n", li, cos(li), li, sin(li));

  x = drem(10.3435,6.2831852);

  printf("drem(10.3435,6.2831852) = %g\n", x);

  x = drem(-10.3435,6.2831852);

  printf("drem(-10.3435,6.2831852) = %g\n", x);

  x = drem(-10.3435,-6.2831852);

  printf("drem(-10.3435,-6.2831852) = %g\n", x);

  x = drem(10.3435,-6.2831852);

  printf("drem(10.3435,-6.2831852) = %g\n", x);


  printf("x%8.6gx\n", .5);
  printf("x%-8.6gx\n", .5);
  printf("x%6.6gx\n", .5);

  {
    double x = atof ("-1e-17-");
    printf ("%g %c= %g %s!\n",
	    x,
	    x == -1e-17 ? '=' : '!',
	    -1e-17,
	    x == -1e-17 ? "Worked" : "Failed");
  }

  print_trig_stuff ();

  return 0;
}


#define PI 3.14159265358979323846264338327

const double RAD[5] = { 0, PI/2, PI, (3*PI)/2, 2*PI };
const int    DEG[5] = { 0, 90, 180, 360 };

#define PRINT_IT_1_ARG(_func, _arg, _value) \
    (_value) = (_func)((_arg)); \
    if (errno) { \
      errno = 0; \
      printf("%s = ERROR %s\n", #_func, strerror(errno)); \
    } else \
      printf("%s(%g) = %g\n", #_func, _arg, (_value)); \

#define PRINT_IT_2_ARG(_func, _arg1, _arg2, _value) \
    (_value) = (_func)((_arg1),(_arg2)); \
    if (errno) { \
      errno = 0; \
      printf("%s = ERROR %s\n", #_func, strerror(errno)); \
    } else \
      printf("%s(%g, %g) = %g\n", #_func, _arg1, _arg2, (_value)); \

void
DEFUN_VOID (print_trig_stuff)
{
  double value, arg1, arg2;
  int i;

  puts ("\n\nMath Test");

  errno = 0;			/* automatically reset on error condition */
  for (i=0; i<4; i++)
    {
      PRINT_IT_1_ARG (sin, RAD[i], value);
      PRINT_IT_1_ARG (cos, RAD[i], value);
      PRINT_IT_1_ARG (tan, RAD[i], value);
      PRINT_IT_1_ARG (asin, RAD[i], value);
      PRINT_IT_1_ARG (acos, RAD[i], value);
      PRINT_IT_1_ARG (atan, RAD[i], value);
      PRINT_IT_2_ARG (atan2, RAD[i], -RAD[i % 4], value);
    }

  arg1 = 16;
  arg2 = 3;
  PRINT_IT_1_ARG (exp, arg1, value);
  PRINT_IT_1_ARG (log, arg1, value);
  PRINT_IT_1_ARG (log10, arg1, value);
  PRINT_IT_2_ARG (pow, arg1, arg2, value);
  PRINT_IT_1_ARG (sqrt, arg1, value);
  PRINT_IT_1_ARG (cbrt, arg1, value);
  PRINT_IT_2_ARG (hypot, arg1, arg2, value);
  PRINT_IT_1_ARG (expm1, arg1, value);
  PRINT_IT_1_ARG (log1p, arg1, value);
  PRINT_IT_1_ARG (sinh, arg1, value);
  PRINT_IT_1_ARG (cosh, arg1, value);
  PRINT_IT_1_ARG (tanh, arg1, value);
  PRINT_IT_1_ARG (asinh, arg1, value);
  PRINT_IT_1_ARG (acosh, arg1, value);
  PRINT_IT_1_ARG (atanh, arg1, value);
}
