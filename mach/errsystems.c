/* XXX temp kludge: this file should be generated, but a gcc bug screws it. */

#include <mach/error.h>
#include <errorlib.h>
#define static static const
#include "../sysdeps/mach/hurd/err_hurd.sub"
#include "err_us.sub"
#include "err_server.sub"
#include "err_mach.sub"
#include "err_kern.sub"
#include "err_ipc.sub"
#include "err_boot.sub"


const struct error_system __mach_error_systems[err_max_system + 1] =
  {
    /* 0 [err_get_system (err_kern)] = */ { errlib_count (err_kern_sub),"(system kern) error with unknown subsystem", err_kern_sub },
    /* 1 [err_get_system (err_us)] = */ { errlib_count (err_us_sub),"(system us) error with unknown subsystem", err_us_sub },
    /* 2 [err_get_system (err_server)] = */ { errlib_count (err_server_sub),"(system server) error with unknown subsystem", err_server_sub },
    /* 3 [err_get_system (err_ipc)] = */ { errlib_count (err_ipc_sub),"(system ipc) error with unknown subsystem", err_ipc_sub },
    /* 4 [err_get_system (err_mach_ipc)] = */ { errlib_count (err_mach_ipc_sub),"(system mach_ipc) error with unknown subsystem", err_mach_ipc_sub },
    /* 5 [err_get_system (err_bootstrap)] = */ { errlib_count (err_bootstrap_sub),"(system bootstrap) error with unknown subsystem", err_bootstrap_sub },
 /* [6..15] */ {},{},{},{},{},{},{},{},{},{},
    /* 16==0x10 [err_get_system (err_hurd)] = */ { errlib_count (err_hurd_sub),"(system hurd) error with unknown subsystem", err_hurd_sub },
  };

const int __mach_error_system_count = errlib_count (__mach_error_systems);
