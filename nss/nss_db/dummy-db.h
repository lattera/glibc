#include <stdint.h>

#include "nss_db.h"

/* This file contains dummy definitions of the DB structure of the
   Berkeley DB.  We are only interested in the function pointers since
   this is the interface to the database.  Unfortunately the structure
   changed over time and we have to take this into account.  */

/* The values to select the database type are unchanged over the version.
   Define only what we really need.  */
#define DB_BTREE	(1)

/* Permission flags are also not changed.  */
#define DB_RDONLY	0x010000


/* This is for the db-2.x version up to 2.x.y.  We use the name `db24' since
   this is the version which was shipped with glibc 2.1.  */
struct db24
{
  void	*mutexp;
  enum { dummy24 } type;
  void *dbenv;
  void *mp_dbenv;
  void *master;
  void *internal;
  void *mp;
  void *mpf;
  struct
  {
    void *tqh_first;
    void **tqh_last;
  } curs_queue;
  struct {
    void *lh_first;
  } handleq;
  struct {
    void *le_next;
    void **le_prev;
  } links;
  uint32_t log_fileid;
  void *txn;
  uint32_t locker;
  struct db24_dbt {
    void *data;
    uint32_t size;
    uint32_t ulen;
    uint32_t dlen;
    uint32_t doff;
    uint32_t flags;
  } lock_dbt;
  struct{
    uint32_t pgno;
    uint8_t fileid[20];
  } lock;
  size_t pgsize;
  void *db_malloc;
  /* Functions. */
  int (*close) (void *, uint32_t);
  int (*cursor) (void *, void *, void **);
  int (*del) (void *, void *, DBT *, uint32_t);
  int (*fd) (void *, int *);
  int (*get) (void *, void *, DBT *, DBT *, uint32_t);
  int (*put) (void *, void *, DBT *, DBT *, uint32_t);
  int (*stat) (void *, void *, void *(*)(size_t), uint32_t);
  int (*sync) (void *, uint32_t);
};


/* Version 2.7, slightly incompatible with version 2.4.  */
struct db27
{
  void	*mutexp;
  enum { dummy27 } type;
  int byteswapped;
  void *dbenv;
  void *mp_dbenv;
  void *internal;
  void *mp;
  void *mpf;
  struct
  {
    void *tqh_first;
    void **tqh_last;
  } free_queue;
  struct
  {
    void *tqh_first;
    void **tqh_last;
  } active_queue;
  struct {
    void *lh_first;
  } handleq;
  uint8_t fileid[20];
  uint32_t log_fileid;
  size_t pgsize;
  void *db_malloc;
  void *dup_compare;
  void *h_hash;
  /* Functions. */
  int (*am_close) (void *);
  int (*close) (void *, uint32_t);
  int (*cursor) (void *, void *, void **, uint32_t);
  int (*del) (void *, void *, DBT *, uint32_t);
  int (*fd) (void *, int *);
  int (*get) (void *, void *, DBT *, DBT *, uint32_t);
  int (*put) (void *, void *, DBT *, DBT *, uint32_t);
  int (*stat) (void *, void *, void *(*)(size_t), uint32_t);
  int (*sync) (void *, uint32_t);
};
