/* We don't need the usual code since we are using the AIX crt code.  */

/* This function is called in the AIX crt0.  */
void
__mod_init (void)
{
  /* XXX What has to be done?  */
}

/* This variable is reference in the AIX crt0 code.
   XXX Since I don't know how it is used make it a pointer to a function.  */
void *__crt0v = __mod_init;


/* XXX Another weird function from the C library.  I have no idea what
   it does but it is needed by libgcc.  */
void
_savef14 (void)
{
}
