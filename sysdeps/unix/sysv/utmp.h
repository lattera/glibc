#ifndef _UTMP_H

#define _UTMP_H	1

#include <time.h>

struct utmp
  {
#define	ut_name	ut_user
    char ut_user[8];
    char ut_id[4];
    char ut_line[12];
    short ut_pid;
    short ut_type;
    struct exit_status
      {
	short e_termination;
	short e_exit;
      } ut_exit;
    time_t ut_time;
  };

#endif /* utmp.h.  */
