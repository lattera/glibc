#include <ansidecl.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int
DEFUN_VOID(main)
{
  uid_t me;
  struct passwd *my_passwd;
  struct group *my_group;
  char **members;

  me = getuid ();
  my_passwd = getpwuid (me);
  if (my_passwd == NULL)
    perror ("getpwuid");
  else
    {
      printf ("My login name is %s.\n", my_passwd->pw_name);
      printf ("My uid is %d.\n", (int)(my_passwd->pw_uid));
      printf ("My home directory is %s.\n", my_passwd->pw_dir);
      printf ("My default shell is %s.\n", my_passwd->pw_shell);

      my_group = getgrgid (my_passwd->pw_gid);
      if (my_group == NULL)
	perror ("getgrgid");
      else
	{
	  printf ("My default group is %s (%d).\n",
		  my_group->gr_name, (int)(my_passwd->pw_gid));
	  printf ("The members of this group are:\n");
	  for (members = my_group->gr_mem; *members != NULL; ++members)
	    printf ("  %s\n", *members);
	}
    }

  exit (my_passwd && my_group ? EXIT_SUCCESS : EXIT_FAILURE);
}



