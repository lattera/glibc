#include_next <pthreadP.h>
#ifndef _PTHREADP_H_HPPA_
#define _PTHREADP_H_HPPA_ 1

/* Internal cond functions.  */
extern int __pthread_cond_broadcast_internal (pthread_cond_t *cond);
extern int __pthread_cond_destroy_internal (pthread_cond_t *cond);
extern int __pthread_cond_init_internal (pthread_cond_t *cond,
                                        const pthread_condattr_t *cond_attr);
extern int __pthread_cond_signal_internal (pthread_cond_t *cond);
extern int __pthread_cond_timedwait_internal (pthread_cond_t *cond,
                                             pthread_mutex_t *mutex,
                                             const struct timespec *abstime);
extern int __pthread_cond_wait_internal (pthread_cond_t *cond,
                                        pthread_mutex_t *mutex);
#endif
