/* Finalizer module for ELF shared C library.  This provides terminating
   null pointer words in the `.ctors' and `.dtors' sections.  */

static void (*const __CTOR_END__[1]) (void)
     __attribute__ ((used, section (".ctors")))
     = { 0 };
static void (*const __DTOR_END__[1]) (void)
     __attribute__ ((used, section (".dtors")))
     = { 0 };

#ifdef HAVE_DWARF2_UNWIND_INFO
/* Terminate the frame unwind info section with a 4byte 0 as a sentinel;
   this would be the 'length' field in a real FDE.  */

typedef unsigned int ui32 __attribute__ ((mode (SI)));
static ui32 __FRAME_END__[1]
     __attribute__ ((used, section (".eh_frame")))
     = { 0 };
#endif
