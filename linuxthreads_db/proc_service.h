typedef enum {
  PS_OK,          /* generic "call succeeded" */
  PS_ERR,         /* generic. */
  PS_BADPID,      /* bad process handle */
  PS_BADLID,      /* bad lwp identifier */
  PS_BADADDR,     /* bad address */
  PS_NOSYM,       /* p_lookup() could not find given symbol */
        PS_NOFREGS
  /*
   * FPU register set not available for given
   * lwp
   */
}       ps_err_e;

typedef unsigned long paddr_t;




struct ps_prochandle;		/* user defined. */


extern ps_err_e ps_pdread(struct ps_prochandle *,
                        psaddr_t, void *, size_t);
extern ps_err_e ps_pdwrite(struct ps_prochandle *,
                        psaddr_t, const void *, size_t);
extern ps_err_e ps_ptread(struct ps_prochandle *,
                        psaddr_t, void *, size_t);
extern ps_err_e ps_ptwrite(struct ps_prochandle *,
                        psaddr_t, const void *, size_t);

extern ps_err_e ps_pglobal_lookup(struct ps_prochandle *,
        const char *object_name, const char *sym_name, psaddr_t *sym_addr);


extern ps_err_e ps_lgetregs(struct ps_prochandle *,
                        lwpid_t, prgregset_t);
extern ps_err_e ps_lsetregs(struct ps_prochandle *,
                        lwpid_t, const prgregset_t);
extern ps_err_e ps_lgetfpregs(struct ps_prochandle *,
                        lwpid_t, prfpregset_t *);
extern ps_err_e ps_lsetfpregs(struct ps_prochandle *,
                        lwpid_t, const prfpregset_t *);

