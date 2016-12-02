#ifndef __sigval_t_defined
#define __sigval_t_defined

/* Type for data associated with a signal.  */
union sigval
{
  int sival_int;
  void *sival_ptr;
};

typedef union sigval sigval_t;

#endif
