/* Private header for thread debug library.  */
#ifndef _THREAD_DBP_H
#define _THREAD_DBP_H	1

#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "proc_service.h"
#include "thread_db.h"
#include "../nptl/pthreadP.h"
#include <list.h>


/* Indeces for the symbol names.  */
enum
  {
    SYM_PTHREAD_THREADS_EVENTS = 0,
    SYM_PTHREAD_LAST_EVENT,
    SYM_PTHREAD_NTHREADS,
    SYM_PTHREAD_STACK_USED,
    SYM_PTHREAD_STACK_USER,
    SYM_PTHREAD_KEYS,
    SYM_PTHREAD_KEYS_MAX,
    SYM_PTHREAD_SIZEOF_DESCR,
    SYM_PTHREAD_CREATE_EVENT,
    SYM_PTHREAD_DEATH_EVENT,
    SYM_PTHREAD_VERSION,
    SYM_NUM_MESSAGES
  };


/* Comment out the following for less verbose output.  */
#ifndef NDEBUG
# define LOG(c) if (__td_debug) __libc_write (2, c "\n", strlen (c "\n"))
extern int __td_debug;
#else
# define LOG(c)
#endif


/* Handle for a process.  This type is opaque.  */
struct td_thragent
{
  /* Delivered by the debugger and we have to pass it back in the
     proc callbacks.  */
  struct ps_prochandle *ph;

  /* Some cached information.  */

  /* Lists of running threads.  */
  psaddr_t stack_used;
  psaddr_t stack_user;

  /* Address of the `pthread_keys' array.  */
  struct pthread_key_struct *keys;

  /* Maximum number of thread-local data keys.  */
  int pthread_keys_max;

  /* Size of 2nd level array for thread-local data keys.  */
  int pthread_key_2ndlevel_size;

  /* Sizeof struct _pthread_descr_struct.  */
  int sizeof_descr;

  /* Pointer to the `__pthread_threads_events' variable in the target.  */
  psaddr_t pthread_threads_eventsp;

  /* Pointer to the `__pthread_last_event' variable in the target.  */
  psaddr_t pthread_last_event;

  /* List head the queue agent structures.  */
  list_t list;
};


/* List of all known descriptors.  */
extern list_t __td_agent_list;


/* Function used to test for correct thread agent pointer.  */
static inline bool
ta_ok (const td_thragent_t *ta)
{
  list_t *runp;

  list_for_each (runp, &__td_agent_list)
    if (list_entry (runp, td_thragent_t, list) == ta)
      return true;

  return false;
}


/* Internal wrapper around ps_pglobal_lookup.  */
extern int td_lookup (struct ps_prochandle *ps, int idx, psaddr_t *sym_addr);

#endif /* thread_dbP.h */
