#ifndef _MACH_MACH_TRAPS_H
#include_next <mach/mach_traps.h>

extern mach_port_t __mach_reply_port (void) attribute_hidden;
extern mach_port_t __mach_thread_self (void);
extern mach_port_t (__mach_task_self) (void);
extern mach_port_t (__mach_host_self) (void) attribute_hidden;
extern boolean_t __swtch (void) attribute_hidden;
extern boolean_t __swtch_pri (int priority) attribute_hidden;
kern_return_t __thread_switch (mach_port_t new_thread,
			     int option, mach_msg_timeout_t option_time) attribute_hidden;
kern_return_t __evc_wait (unsigned int event) attribute_hidden;
#endif
