/* Finalizer module for ELF shared C library.  This provides terminating
   null pointer words in the `.ctors' and `.dtors' sections.  */

static void (*const __CTOR_END__[1]) (void)
     __attribute__ ((unused, section (".ctors")))
     = { 0 };
static void (*const __DTOR_END__[1]) (void)
     __attribute__ ((unused, section (".dtors")))
     = { 0 };
