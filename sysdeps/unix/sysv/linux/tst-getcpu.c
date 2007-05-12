#include <errno.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>


static int
do_test (void)
{
  cpu_set_t cs;
  if (sched_getaffinity (getpid (), sizeof (cs), &cs) != 0)
    {
      printf ("getaffinity failed: %m\n");
      return 1;
    }

  int result = 0;
  int cpu = 0;
  while (CPU_COUNT (&cs) != 0)
    {
      if (CPU_ISSET (cpu, &cs))
	{
	  cpu_set_t cs2;
	  CPU_ZERO (&cs2);
	  CPU_SET (cpu, &cs2);
	  if (sched_setaffinity (getpid (), sizeof (cs2), &cs2) != 0)
	    {
	      printf ("setaffinity(%d) failed: %m\n", cpu);
	      result = 1;
	    }
	  else
	    {
	      int cpu2 = sched_getcpu ();
	      if (cpu2 == -1 && errno == ENOSYS)
		{
		  puts ("getcpu syscall not implemented");
		  return 0;
		}
	      if (cpu2 != cpu)
		{
		  printf ("getcpu results %d not possible\n", cpu2);
		  result = 1;
		}
	    }
	  CPU_CLR (cpu, &cs);
	}
      ++cpu;
    }

  return result;
}

#define TEST_FUNCTION do_test ()
#include <test-skeleton.c>
