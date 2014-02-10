#define _GNU_SOURCE
#include <argp.h>
#include <complex.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <gd.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>


#define size_x 320
#define size_y 240


#define PATH "/tmp/s.sockperf"


struct thread_param
{
  unsigned int from;
  unsigned int to;
  unsigned int nserv;
};

struct coord
{
  unsigned int x;
  unsigned int y;
  complex double z;
};


/* We use 64bit values for the times.  */
typedef unsigned long long int hp_timing_t;


static unsigned int nclients = 2;
static unsigned int nservers = 2;

static bool timing;
static int points;


static complex double top_left = -0.7 + 0.2i;
static complex double bottom_right = -0.5 - 0.0i;


static int colors[256];
static gdImagePtr image;
static pthread_mutex_t image_lock;

static int sock;


static void *
client (void *arg)
{
  struct thread_param *param = arg;
  unsigned int cnt;
  unsigned int nserv = param->nserv;
  int clisock[nserv];
  struct pollfd servpoll[nserv];
  struct sockaddr_un servaddr;
  socklen_t servlen;
  struct coord c;

  bool new_coord (void)
    {
      if (cnt >= param->to)
	return false;

      unsigned int row = cnt / size_x;
      unsigned int col = cnt % size_x;

      c.x = col;
      c.y = row;
      c.z = (top_left
	     + ((col
		 * (creal (bottom_right) - creal (top_left))) / size_x)
	     + (_Complex_I * (row * (cimag (bottom_right) - cimag (top_left)))
		/ size_y));

      ++cnt;

      return true;
    }


  for (cnt = 0; cnt < nserv; ++cnt)
    {
      servpoll[cnt].fd = socket (AF_UNIX, SOCK_STREAM, 0);
      if (clisock < 0)
	{
	  puts ("cannot create socket in client");
	  return NULL;
	}

      memset (&servaddr, '\0', sizeof (servaddr));
      servaddr.sun_family = AF_UNIX;
      strncpy (servaddr.sun_path, PATH, sizeof (servaddr.sun_path));
      servlen = offsetof (struct sockaddr_un, sun_path) + strlen (PATH) + 1;


      int err;
      while (1)
	{
	  err = TEMP_FAILURE_RETRY (connect (servpoll[cnt].fd, &servaddr,
					     servlen));
	  if (err != -1 || errno != ECONNREFUSED)
	    break;

	  pthread_yield ();
	}

      if (err == -1)
	{
	  printf ("cannot connect: %m (%d)\n", errno);
	  exit (1);
	}

      servpoll[cnt].events = POLLOUT;
      servpoll[cnt].revents = 0;
    }

  cnt = param->from;

  new_coord ();
  bool z_valid = true;

  while (1)
    {
      int i;
      int n = poll (servpoll, nserv, -1);
      if (n == -1)
	{
	  puts ("poll returned error");
	  break;
	}

      bool cont = false;
      for (i = 0; i < nserv && n > 0; ++i)
	if (servpoll[i].revents != 0)
	  {
	    if (servpoll[i].revents == POLLIN)
	      {
		unsigned int vals[3];
		if (TEMP_FAILURE_RETRY (read (servpoll[i].fd, &vals,
					      sizeof (vals)))
		    != sizeof (vals))
		  {
		    puts ("read error in client");
		    return NULL;
		  }

		pthread_mutex_lock (&image_lock);

		gdImageSetPixel (image, vals[0], vals[1], vals[2]);
		++points;

		pthread_mutex_unlock (&image_lock);

		servpoll[i].events = POLLOUT;
	      }
	    else
	      {
		if (servpoll[i].revents != POLLOUT)
		  printf ("revents: %hd != POLLOUT ???\n",
			  servpoll[i].revents);

		if (z_valid)
		  {
		    if (TEMP_FAILURE_RETRY (write (servpoll[i].fd, &c,
						   sizeof (c))) != sizeof (c))
		      {
			puts ("write error in client");
			return NULL;
		      }
		    cont = true;
		    servpoll[i].events = POLLIN;

		    z_valid = new_coord ();
		    if (! z_valid)
		      /* No more to do.  Clear the event fields.  */
		      for (i = 0; i < nserv; ++i)
			if (servpoll[i].events == POLLOUT)
			  servpoll[i].events = servpoll[i].revents = 0;
		  }
		else
		  servpoll[i].events = servpoll[i].revents = 0;
	      }

	    --n;
	  }
	else if (servpoll[i].events != 0)
	  cont = true;

      if (! cont && ! z_valid)
	break;
    }

  c.x = 0xffffffff;
  c.y = 0xffffffff;
  for (cnt = 0; cnt < nserv; ++cnt)
    {
      TEMP_FAILURE_RETRY (write (servpoll[cnt].fd, &c, sizeof (c)));
      close (servpoll[cnt].fd);
    }

  return NULL;
}


static void *
server (void *arg)
{
  struct sockaddr_un cliaddr;
  socklen_t clilen;
  int clisock = TEMP_FAILURE_RETRY (accept (sock, &cliaddr, &clilen));

  if (clisock == -1)
    {
      puts ("accept failed");
      return NULL;
    }

  while (1)
    {
      struct coord c;

      if (TEMP_FAILURE_RETRY (read (clisock, &c, sizeof (c))) != sizeof (c))
	{
	  printf ("server read failed: %m (%d)\n", errno);
	  break;
	}

      if (c.x == 0xffffffff && c.y == 0xffffffff)
	break;

      unsigned int rnds = 0;
      complex double z = c.z;
      while (cabs (z) < 4.0)
	{
	  z = z * z - 1;
	  if (++rnds == 255)
	    break;
	}

      unsigned int vals[3] = { c.x, c.y, rnds };
      if (TEMP_FAILURE_RETRY (write (clisock, vals, sizeof (vals)))
	  != sizeof (vals))
	{
	  puts ("server write error");
	  return NULL;
	}
    }

  close (clisock);

  return NULL;
}


static const char *outfilename = "test.png";


static const struct argp_option options[] =
  {
    { "clients", 'c', "NUMBER", 0, "Number of client threads" },
    { "servers", 's', "NUMBER", 0, "Number of server threads per client" },
    { "timing", 'T', NULL, 0,
      "Measure time from startup to the last thread finishing" },
    { NULL, 0, NULL, 0, NULL }
  };

/* Prototype for option handler.  */
static error_t parse_opt (int key, char *arg, struct argp_state *state);

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  options, parse_opt
};


int
main (int argc, char *argv[])
{
  int cnt;
  FILE *outfile;
  struct sockaddr_un servaddr;
  socklen_t servlen;
  int remaining;

  /* Parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);


  pthread_t servth[nservers * nclients];
  pthread_t clntth[nclients];
  struct thread_param clntparam[nclients];


  image = gdImageCreate (size_x, size_y);
  if (image == NULL)
    {
      puts ("gdImageCreate failed");
      return 1;
    }

  for (cnt = 0; cnt < 255; ++cnt)
    colors[cnt] = gdImageColorAllocate (image, 256 - cnt, 256 - cnt,
					256 - cnt);
  /* Black.  */
  colors[cnt] = gdImageColorAllocate (image, 0, 0, 0);


  sock = socket (AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    error (EXIT_FAILURE, errno, "cannot create socket");

  memset (&servaddr, '\0', sizeof (servaddr));
  servaddr.sun_family = AF_UNIX;
  strncpy (servaddr.sun_path, PATH, sizeof (servaddr.sun_path));
  servlen = offsetof (struct sockaddr_un, sun_path) + strlen (PATH) + 1;

  if (bind (sock, &servaddr, servlen) == -1)
    error (EXIT_FAILURE, errno, "bind failed");

  listen (sock, SOMAXCONN);

  pthread_mutex_init (&image_lock, NULL);


  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;

  clockid_t cl;
  struct timespec start_time;
  if (timing)
    {
      if (clock_getcpuclockid (0, &cl) != 0
	  || clock_gettime (cl, &start_time) != 0)
	timing = false;
    }

  /* Start the servers.  */
  for (cnt = 0; cnt < nservers * nclients; ++cnt)
    {
      if (pthread_create (&servth[cnt], NULL, server, NULL) != 0)
	{
	  puts ("pthread_create for server failed");
	  exit (1);
	}
    }

  for (cnt = 0; cnt < nclients; ++cnt)
    {
      clntparam[cnt].from = cnt * (size_x * size_y) / nclients;
      clntparam[cnt].to = MIN ((cnt + 1) * (size_x * size_y) / nclients,
			       size_x * size_y);
      clntparam[cnt].nserv = nservers;

      if (pthread_create (&clntth[cnt], NULL, client, &clntparam[cnt]) != 0)
	{
	  puts ("pthread_create for client failed");
	  exit (1);
	}
    }


  /* Wait for the clients.  */
  for (cnt = 0; cnt < nclients; ++cnt)
    if (pthread_join (clntth[cnt], NULL) != 0)
      {
	puts ("client pthread_join failed");
	exit (1);
      }

  /* Wait for the servers.  */
  for (cnt = 0; cnt < nclients * nservers; ++cnt)
    if (pthread_join (servth[cnt], NULL) != 0)
      {
	puts ("server pthread_join failed");
	exit (1);
      }


  if (timing)
    {
      struct timespec end_time;

      if (clock_gettime (cl, &end_time) == 0)
	{
	  end_time.tv_sec -= start_time.tv_sec;
	  end_time.tv_nsec -= start_time.tv_nsec;
	  if (end_time.tv_nsec < 0)
	    {
	      end_time.tv_nsec += 1000000000;
	      --end_time.tv_sec;
	    }

	  printf ("\nRuntime: %lu.%09lu seconds\n%d points computed\n",
		  (unsigned long int) end_time.tv_sec,
		  (unsigned long int) end_time.tv_nsec,
		  points);
	}
    }


  outfile = fopen (outfilename, "w");
  if (outfile == NULL)
    error (EXIT_FAILURE, errno, "cannot open output file '%s'", outfilename);

  gdImagePng (image, outfile);

  fclose (outfile);

  unlink (PATH);

  return 0;
}


/* Handle program arguments.  */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'c':
      nclients = strtoul (arg, NULL, 0);
      break;

    case 's':
      nservers = strtoul (arg, NULL, 0);
      break;

    case 'T':
      timing = true;
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}


static hp_timing_t
get_clockfreq (void)
{
  /* We read the information from the /proc filesystem.  It contains at
     least one line like
	cpu MHz         : 497.840237
     or also
	cpu MHz         : 497.841
     We search for this line and convert the number in an integer.  */
  static hp_timing_t result;
  int fd;

  /* If this function was called before, we know the result.  */
  if (result != 0)
    return result;

  fd = open ("/proc/cpuinfo", O_RDONLY);
  if (__glibc_likely (fd != -1))
    {
      /* XXX AFAIK the /proc filesystem can generate "files" only up
         to a size of 4096 bytes.  */
      char buf[4096];
      ssize_t n;

      n = read (fd, buf, sizeof buf);
      if (__builtin_expect (n, 1) > 0)
	{
	  char *mhz = memmem (buf, n, "cpu MHz", 7);

	  if (__glibc_likely (mhz != NULL))
	    {
	      char *endp = buf + n;
	      int seen_decpoint = 0;
	      int ndigits = 0;

	      /* Search for the beginning of the string.  */
	      while (mhz < endp && (*mhz < '0' || *mhz > '9') && *mhz != '\n')
		++mhz;

	      while (mhz < endp && *mhz != '\n')
		{
		  if (*mhz >= '0' && *mhz <= '9')
		    {
		      result *= 10;
		      result += *mhz - '0';
		      if (seen_decpoint)
			++ndigits;
		    }
		  else if (*mhz == '.')
		    seen_decpoint = 1;

		  ++mhz;
		}

	      /* Compensate for missing digits at the end.  */
	      while (ndigits++ < 6)
		result *= 10;
	    }
	}

      close (fd);
    }

  return result;
}


int
clock_getcpuclockid (pid_t pid, clockid_t *clock_id)
{
  /* We don't allow any process ID but our own.  */
  if (pid != 0 && pid != getpid ())
    return EPERM;

#ifdef CLOCK_PROCESS_CPUTIME_ID
  /* Store the number.  */
  *clock_id = CLOCK_PROCESS_CPUTIME_ID;

  return 0;
#else
  /* We don't have a timer for that.  */
  return ENOENT;
#endif
}


#define HP_TIMING_NOW(Var)	__asm__ __volatile__ ("rdtsc" : "=A" (Var))

/* Get current value of CLOCK and store it in TP.  */
int
clock_gettime (clockid_t clock_id, struct timespec *tp)
{
  int retval = -1;

  switch (clock_id)
    {
    case CLOCK_PROCESS_CPUTIME_ID:
      {

	static hp_timing_t freq;
	hp_timing_t tsc;

	/* Get the current counter.  */
	HP_TIMING_NOW (tsc);

	if (freq == 0)
	  {
	    freq = get_clockfreq ();
	    if (freq == 0)
	      return EINVAL;
	  }

	/* Compute the seconds.  */
	tp->tv_sec = tsc / freq;

	/* And the nanoseconds.  This computation should be stable until
	   we get machines with about 16GHz frequency.  */
	tp->tv_nsec = ((tsc % freq) * UINT64_C (1000000000)) / freq;

	retval = 0;
      }
    break;

    default:
      errno = EINVAL;
      break;
    }

  return retval;
}
