/* Prototype for the setgrent functions we use here.  */
typedef enum nss_status (*set_function) (void);

/* Prototype for the endgrent functions we use here.  */
typedef enum nss_status (*end_function) (void);

/* Prototype for the setgrent functions we use here.  */
typedef enum nss_status (*get_function) (struct group *, char *,
					 size_t, int *);

static enum nss_status
compat_call (service_user *nip, const char *user, gid_t group, long int *start,
	     long int *size, gid_t **groupsp, long int limit, int *errnop)
{
  struct group grpbuf;
  size_t buflen = __sysconf (_SC_GETGR_R_SIZE_MAX);
  char *tmpbuf;
  enum nss_status status;
  set_function setgrent_fct;
  get_function getgrent_fct;
  end_function endgrent_fct;
  gid_t *groups = *groupsp;

  getgrent_fct = __nss_lookup_function (nip, "getgrent_r");
  if (getgrent_fct == NULL)
    return NSS_STATUS_UNAVAIL;

  setgrent_fct = __nss_lookup_function (nip, "setgrent");
  if (setgrent_fct)
    {
      status = DL_CALL_FCT (setgrent_fct, ());
      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  endgrent_fct = __nss_lookup_function (nip, "endgrent");

  tmpbuf = __alloca (buflen);

  do
    {
      while ((status = DL_CALL_FCT (getgrent_fct,
				     (&grpbuf, tmpbuf, buflen, errnop)),
	      status == NSS_STATUS_TRYAGAIN)
	     && *errnop == ERANGE)
        {
          buflen *= 2;
          tmpbuf = __alloca (buflen);
        }

      if (status != NSS_STATUS_SUCCESS)
        goto done;

      if (grpbuf.gr_gid != group)
        {
          char **m;

          for (m = grpbuf.gr_mem; *m != NULL; ++m)
            if (strcmp (*m, user) == 0)
              {
		/* Check whether the group is already on the list.  */
		long int cnt;
		for (cnt = 0; cnt < *start; ++cnt)
		  if (groups[cnt] == grpbuf.gr_gid)
		    break;

		if (cnt == *start)
		  {
		    /* Matches user and not yet on the list.  Insert
		       this group.  */
		    if (__builtin_expect (*start == *size, 0))
		      {
			/* Need a bigger buffer.  */
			gid_t *newgroups;
			long int newsize;

			if (limit > 0 && *size == limit)
			  /* We reached the maximum.  */
			  goto done;

			if (limit <= 0)
			  newsize = 2 * *size;
			else
			  newsize = MIN (limit, 2 * *size);

			newgroups = realloc (groups,
					     newsize * sizeof (*groups));
			if (newgroups == NULL)
			  goto done;
			*groupsp = groups = newgroups;
			*size = newsize;
		      }

		    groups[*start] = grpbuf.gr_gid;
		    *start += 1;
		  }

                break;
              }
        }
    }
  while (status == NSS_STATUS_SUCCESS);

 done:
  if (endgrent_fct)
    DL_CALL_FCT (endgrent_fct, ());

  return NSS_STATUS_SUCCESS;
}
