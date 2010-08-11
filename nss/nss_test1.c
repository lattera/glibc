#include <errno.h>
#include <nss.h>
#include <pthread.h>
#include <string.h>


#define COPY_IF_ROOM(s) \
  ({ size_t len_ = strlen (s) + 1;		\
     char *start_ = cp;				\
     buflen - (cp - buffer) < len_		\
     ? NULL					\
     : (cp = mempcpy (cp, s, len_), start_); })


/* Password handling.  */
#include <pwd.h>

static struct passwd pwd_data[] =
  {
#define PWD(u) \
    { .pw_name = (char *) "name" #u, .pw_passwd = (char *) "*", .pw_uid = u,  \
      .pw_gid = 100, .pw_gecos = (char *) "*", .pw_dir = (char *) "*",	      \
      .pw_shell = (char *) "*" }
    PWD (100),
    PWD (30),
    PWD (200),
    PWD (60),
    PWD (20000)
  };
#define npwd_data (sizeof (pwd_data) / sizeof (pwd_data[0]))

static size_t pwd_iter;
#define CURPWD pwd_data[pwd_iter]

static pthread_mutex_t pwd_lock = PTHREAD_MUTEX_INITIALIZER;


enum nss_status
_nss_test1_setpwent (int stayopen)
{
  pwd_iter = 0;
  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_test1_endpwent (void)
{
  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_test1_getpwent_r (struct passwd *result, char *buffer, size_t buflen,
		       int *errnop)
{
  char *cp = buffer;
  int res = NSS_STATUS_SUCCESS;

  pthread_mutex_lock (&pwd_lock);

  if (pwd_iter >= npwd_data)
    res = NSS_STATUS_NOTFOUND;
  else
    {
      result->pw_name = COPY_IF_ROOM (CURPWD.pw_name);
      result->pw_passwd = COPY_IF_ROOM (CURPWD.pw_passwd);
      result->pw_uid = CURPWD.pw_uid;
      result->pw_gid = CURPWD.pw_gid;
      result->pw_gecos = COPY_IF_ROOM (CURPWD.pw_gecos);
      result->pw_dir = COPY_IF_ROOM (CURPWD.pw_dir);
      result->pw_shell = COPY_IF_ROOM (CURPWD.pw_shell);

      if (result->pw_name == NULL || result->pw_passwd == NULL
	  || result->pw_gecos == NULL || result->pw_dir == NULL
	  || result->pw_shell == NULL)
	{
	  *errnop = ERANGE;
	  res = NSS_STATUS_TRYAGAIN;
	}

      ++pwd_iter;
    }

  pthread_mutex_unlock (&pwd_lock);

  return res;
}


enum nss_status
_nss_test1_getpwuid_r (uid_t uid, struct passwd *result, char *buffer,
		       size_t buflen, int *errnop)
{
  for (size_t idx = 0; idx < npwd_data; ++idx)
    if (pwd_data[idx].pw_uid == uid)
      {
	char *cp = buffer;
	int res = NSS_STATUS_SUCCESS;

	result->pw_name = COPY_IF_ROOM (pwd_data[idx].pw_name);
	result->pw_passwd = COPY_IF_ROOM (pwd_data[idx].pw_passwd);
	result->pw_uid = pwd_data[idx].pw_uid;
	result->pw_gid = pwd_data[idx].pw_gid;
	result->pw_gecos = COPY_IF_ROOM (pwd_data[idx].pw_gecos);
	result->pw_dir = COPY_IF_ROOM (pwd_data[idx].pw_dir);
	result->pw_shell = COPY_IF_ROOM (pwd_data[idx].pw_shell);

	if (result->pw_name == NULL || result->pw_passwd == NULL
	    || result->pw_gecos == NULL || result->pw_dir == NULL
	    || result->pw_shell == NULL)
	  {
	    *errnop = ERANGE;
	    res = NSS_STATUS_TRYAGAIN;
	  }

	return res;
      }

  return NSS_STATUS_NOTFOUND;
}


enum nss_status
_nss_test1_getpwnam_r (const char *name, struct passwd *result, char *buffer,
		       size_t buflen, int *errnop)
{
  for (size_t idx = 0; idx < npwd_data; ++idx)
    if (strcmp (pwd_data[idx].pw_name, name) == 0)
      {
	char *cp = buffer;
	int res = NSS_STATUS_SUCCESS;

	result->pw_name = COPY_IF_ROOM (pwd_data[idx].pw_name);
	result->pw_passwd = COPY_IF_ROOM (pwd_data[idx].pw_passwd);
	result->pw_uid = pwd_data[idx].pw_uid;
	result->pw_gid = pwd_data[idx].pw_gid;
	result->pw_gecos = COPY_IF_ROOM (pwd_data[idx].pw_gecos);
	result->pw_dir = COPY_IF_ROOM (pwd_data[idx].pw_dir);
	result->pw_shell = COPY_IF_ROOM (pwd_data[idx].pw_shell);

	if (result->pw_name == NULL || result->pw_passwd == NULL
	    || result->pw_gecos == NULL || result->pw_dir == NULL
	    || result->pw_shell == NULL)
	  {
	    *errnop = ERANGE;
	    res = NSS_STATUS_TRYAGAIN;
	  }

	return res;
      }

  return NSS_STATUS_NOTFOUND;
}
