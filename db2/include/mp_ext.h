/* DO NOT EDIT: automatically built by dist/distrib. */
int __memp_bhwrite
    __P((DB_MPOOL *, MPOOLFILE *, BH *, int *, int *));
int __memp_pgread __P((DB_MPOOLFILE *, BH *, int));
int __memp_pgwrite __P((DB_MPOOLFILE *, BH *, int *, int *));
int __memp_pg __P((DB_MPOOLFILE *, BH *, int));
void __memp_bhfree __P((DB_MPOOL *, MPOOLFILE *, BH *, int));
int __memp_fopen __P((DB_MPOOL *, const char *, int, int,
   int, size_t, int, DBT *, u_int8_t *, int, DB_MPOOLFILE **));
void __memp_debug __P((DB_MPOOL *, FILE *, int));
int __memp_ralloc __P((DB_MPOOL *, size_t, size_t *, void *));
int __memp_ropen
   __P((DB_MPOOL *, const char *, size_t, int, int));
int __memp_rclose __P((DB_MPOOL *));
