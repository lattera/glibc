#include <stdio.h>
#include <bits/libc-lock.h>
#include <rpc/rpc.h>
#include <assert.h>

#include <bits/libc-lock.h>
#include <bits/libc-tsd.h>

#ifdef _RPC_THREAD_SAFE_


/* Variable used in non-threaded applications.  */
static struct rpc_thread_variables __libc_tsd_RPC_VARS_mem;
static struct rpc_thread_variables *__libc_tsd_RPC_VARS_data =
     &__libc_tsd_RPC_VARS_mem;

/*
 * Task-variable destructor
 */
void
__rpc_thread_destroy (void)
{
	struct rpc_thread_variables *tvp = __rpc_thread_variables();

	if (tvp != NULL) {
		__rpc_thread_svc_cleanup ();
		__rpc_thread_clnt_cleanup ();
		__rpc_thread_key_cleanup ();
		free (tvp->authnone_private_s);
		free (tvp->clnt_perr_buf_s);
		free (tvp->clntraw_private_s);
		free (tvp->svcraw_private_s);
		free (tvp->authdes_cache_s);
		free (tvp->authdes_lru_s);
		free (tvp);
	}
}


struct rpc_thread_variables *
__rpc_thread_variables (void)
{
	struct rpc_thread_variables *tvp;

	tvp = __libc_tsd_get (RPC_VARS);
	if (tvp == NULL) {
		tvp = calloc (1, sizeof *tvp);
		if (tvp != NULL)
			__libc_tsd_set (RPC_VARS, tvp);
		else
			tvp = __libc_tsd_RPC_VARS_data;
	}
	return tvp;
}
#endif /* _RPC_THREAD_SAFE_ */
