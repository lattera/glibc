#include <unistd.h>

static void init (void) __attribute__ ((constructor));

static void
init (void)
{
  write (STDOUT_FILENO, "DSO init\n", 9);
}

static void fini (void) __attribute__ ((destructor));

static void
fini (void)
{
  write (STDOUT_FILENO, "DSO fini\n", 9);
}

static void
init_0 (void)
{
  write (STDOUT_FILENO, "DSO init array 0\n", 17);
}

static void
init_1 (void)
{
  write (STDOUT_FILENO, "DSO init array 1\n", 17);
}

static void
init_2 (void)
{
  write (STDOUT_FILENO, "DSO init array 2\n", 17);
}

void (*const init_array []) (void)
     __attribute__ ((section (".init_array"), aligned (sizeof (void *)))) =
{
  &init_0,
  &init_1,
  &init_2
};

static void
fini_0 (void)
{
  write (STDOUT_FILENO, "DSO fini array 0\n", 17);
}

static void
fini_1 (void)
{
  write (STDOUT_FILENO, "DSO fini array 1\n", 17);
}

static void
fini_2 (void)
{
  write (STDOUT_FILENO, "DSO fini array 2\n", 17);
}

void (*const fini_array []) (void)
     __attribute__ ((section (".fini_array"), aligned (sizeof (void *)))) =
{
  &fini_0,
  &fini_1,
  &fini_2
};
