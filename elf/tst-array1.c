#include <unistd.h>

static void init (void) __attribute__ ((constructor));

static void
init (void)
{
  write (STDOUT_FILENO, "init\n", 5);
}

static void fini (void) __attribute__ ((destructor));

static void
fini (void)
{
  write (STDOUT_FILENO, "fini\n", 5);
}

static void
preinit_0 (void)
{
  write (STDOUT_FILENO, "preinit array 0\n", 16);
}

static void
preinit_1 (void)
{
  write (STDOUT_FILENO, "preinit array 1\n", 16);
}

static void
preinit_2 (void)
{
  write (STDOUT_FILENO, "preinit array 2\n", 16);
}

void (*const preinit_array []) (void)
     __attribute__ ((section (".preinit_array"), aligned (sizeof (void *)))) =
{
  &preinit_0,
  &preinit_1,
  &preinit_2
};

static void
init_0 (void)
{
  write (STDOUT_FILENO, "init array 0\n", 13);
}

static void
init_1 (void)
{
  write (STDOUT_FILENO, "init array 1\n", 13);
}

static void
init_2 (void)
{
  write (STDOUT_FILENO, "init array 2\n", 13);
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
  write (STDOUT_FILENO, "fini array 0\n", 13);
}

static void
fini_1 (void)
{
  write (STDOUT_FILENO, "fini array 1\n", 13);
}

static void
fini_2 (void)
{
  write (STDOUT_FILENO, "fini array 2\n", 13);
}

void (*const fini_array []) (void)
     __attribute__ ((section (".fini_array"), aligned (sizeof (void *)))) =
{
  &fini_0,
  &fini_1,
  &fini_2
};

int
main (void)
{
  return 0;
}
