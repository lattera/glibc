/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef	__RPCSVC_NISLIB_H__
#define	__RPCSVC_NISLIB_H__

#include <features.h>

__BEGIN_DECLS

typedef const char *const_nis_name;

/* nis_names: These functions are used to locate and manipulate all NIS+
 * objects except the NIS+ entry objects.
 *
 * nis_lookup (name, flags) resolves a NIS+ name and returns a copy of
 *                          that object  from a NIS+ server.
 *    const nis_name name: name of the object to be resolved
 *    u_long flags: logically ORing zero or more flags (FOLLOW_LINKS,
 *                  HARD_LOOKUP, [NO_CACHE], MASTER_ONLY, EXPAND_NAME)
 *
 * nis_add (name, obj) adds objects to the NIS+ namespace.
 *    const nis_name name: fully qualified NIS+ name.
 *    const nis_object *obj: object members zo_name and zo_domain will be
 *                           constructed from name.
 *
 * nis_remove (name, obj) removes objects from the NIS+ namespace.
 *    const nis_name name: fully qualified NIS+ name.
 *    const nis_object *obj: if not NULL, it is assumed to point to a copy
 *                           of the object being removed. In this case, if
 *                           the object on the server does not have the same
 *                           object identifier as the  object  being  passed,
 *                           the operation will fail with the NIS_NOTSAMEOBJ
 *                           error.
 *
 * nis_modify (name, obj) can change specific attributes of an object
 *                        that already exists in the namespace.
 */
extern nis_result *nis_lookup __P ((const_nis_name name, u_long flags));
extern nis_result *nis_add __P ((const_nis_name name, const nis_object *obj));
extern nis_result *nis_remove __P ((const_nis_name name,
				    const nis_object *obj));
extern nis_result *nis_modify __P ((const_nis_name name,
				    const nis_object *obj));

/* nis_tables: These functions are used to search and modify NIS+ tables.
 *
 * nis_list (table_name, flags, callback(table_name, obj, userdata), userdata)
 *           search a table in the NIS+ namespace.
 *    const nis_name table_name: indexed name ([xx=yy],table.dir)
 *    u_long flags: logically ORing one or more flags (FOLLOW_LINKS,
 *                  [FOLLOW_PATH], HARD_LOOKUP, [ALL_RESULTS], [NO_CACHE],
 *                  MASTER_ONLY, EXPAND_NAME, RETURN_RESULT)
 *    callback(): callback is an optional pointer to a function that will
 *                process the ENTRY type objects that are returned from the
 *                search. If this pointer is NULL, then all entries that match
 *                the search criteria are returned in the nis_result structure,
 *                otherwise  this  function  will  be  called once for each
 *                entry returned.
 *    void *userdata: passed to callback function along with the returned
 *                    entry object.
 *
 * nis_add_entry (table_name, obj, flags) will add the NIS+ object to the
 *                                        NIS+ table_name.
 *    const nis_name table_name
 *    const nis_object *obj
 *    u_long flags: 0, ADD_OVERWRITE, RETURN_RESULT
 *
 * nis_modify_entry (name, obj, flags) modifies an object identified by name.
 *    const nis_name name: object identifier
 *    const nis_object *obj: should point to an entry with the EN_MODIFIED
 *                           flag set in each column that contains new
 *                           information.
 *    u_long flags: 0, MOD_SAMEOBJ, RETURN_RESULT
 *
 * nis_remove_entry (table_name, obj, flags) removes a set of entries
 *                                 identified by table_name from the table.
 *    const nis_name table_name: indexed NIS+ name
 *    const nis_object *obj: if obj is non-null, it is presumed to point to
 *                           a cached copy of the entry. When the removal is
 *                           attempted, and the object that would be removed
 *                           is not the same as the cached object pointed to
 *                           by object then the operation will fail with an
 *                           NIS_NOTSAMEOBJ error
 *    u_long flags: 0, REM_MULTIPLE
 *
 * nis_first_entry (table_name) fetches entries from a table one at a time.
 *    const nis_name table_name
 *
 * nis_next_entry (table_name, cookie) retrieves the "next" entry from a
 *                                     table specified by table_name.
 *    const nis_name table_name:
 *    const netobj *cookie: The value of cookie from the nis_result structure
 *                          form the previous call.
 */
extern nis_result *nis_list __P ((const_nis_name name, u_long flags,
				  int (*callback)(const_nis_name table_name,
						  const nis_object *obj,
						  const void *userdata),
				  const void *userdata));
extern nis_result *nis_add_entry __P ((const_nis_name table_name,
				       const nis_object *obj, u_long flags));
extern nis_result *nis_modify_entry __P ((const_nis_name name,
					  const nis_object *obj,
					  u_long flags));
extern nis_result *nis_remove_entry __P ((const_nis_name table_name,
					  const nis_object *obj,
					  u_long flags));
extern nis_result *nis_first_entry __P ((const_nis_name table_name));
extern nis_result *nis_next_entry __P ((const_nis_name table_name,
					const netobj *cookie));
/*
** nis_server
*/
extern nis_error nis_mkdir __P ((const_nis_name dirname,
				 const nis_server *machine));
extern nis_error nis_rmdir __P ((const_nis_name dirname,
				 const nis_server *machine));
extern nis_error nis_servstate __P ((const nis_server *machine,
				     const nis_tag *tags, int numtags,
				     nis_tag **result));
extern nis_error nis_stats __P ((const nis_server *machine,
				 const nis_tag *tags, int numtags,
				 nis_tag **result));
extern void nis_freetags __P ((nis_tag *tags, int numtags));
extern nis_server **nis_getservlist __P ((const_nis_name dirname));
extern void nis_freeservlist __P ((nis_server **machines));

/*
** nis_subr
*/
extern nis_name nis_leaf_of __P ((const_nis_name name));
extern nis_name nis_leaf_of_r __P ((const_nis_name name, char *buffer,
				    size_t buflen));
extern nis_name nis_name_of __P ((const_nis_name name));
extern nis_name nis_name_of_r __P ((const_nis_name name, char *buffer,
				    size_t buflen));
extern nis_name nis_domain_of __P ((const_nis_name name));
extern nis_name nis_domain_of_r __P ((const_nis_name name, char *buffer,
				      size_t buflen));
extern nis_name *nis_getnames __P ((const_nis_name name));
extern void nis_freenames __P ((nis_name *namelist));
extern name_pos nis_dir_cmp __P ((const_nis_name n1, const_nis_name n2));
extern nis_object *nis_clone_object __P ((const nis_object *src,
					  nis_object *dest));
extern void nis_destroy_object __P ((nis_object *obj));
extern void nis_print_object __P ((const nis_object *obj));

/*
** nis_local_names
*/
extern nis_name nis_local_group __P ((void));
extern nis_name nis_local_directory __P ((void));
extern nis_name nis_local_principal __P ((void));
extern nis_name nis_local_host __P ((void));

/*
** nis_error
*/
extern const char *nis_sperrno __P ((const nis_error status));
extern void nis_perror __P ((const nis_error status, const char *label));
extern void nis_lerror __P ((const nis_error status, const char *label));
extern char *nis_sperror __P ((const nis_error status, const char *label));
extern char *nis_sperror_r __P ((const nis_error status, const char *label,
				 char *buffer, size_t buflen));
/*
** nis_groups
*/
extern bool_t nis_ismember __P ((const_nis_name principal,
				 const_nis_name group));
extern nis_error nis_addmember __P ((const_nis_name member,
				     const_nis_name group));
extern nis_error nis_removemember __P ((const_nis_name member,
					const_nis_name group));
extern nis_error nis_creategroup __P ((const_nis_name group, u_long flags));
extern nis_error nis_destroygroup __P ((const_nis_name group));
extern void nis_print_group_entry __P ((const_nis_name group));
extern nis_error nis_verifygroup __P ((const_nis_name group));

/*
** nis_ping
*/
extern void nis_ping __P ((const_nis_name dirname, u_long utime,
			   const nis_object *dirobj));
extern nis_result *nis_checkpoint __P ((const_nis_name dirname));

/*
** nis_print (XXX INTERNAL FUNCTIONS, SHOULD NOT BE USED !!)
*/
extern void nis_print_result __P ((const nis_result *result));
extern void nis_print_rights __P ((u_long rights));
extern void nis_print_directory __P ((const directory_obj *dirobj));
extern void nis_print_group __P ((const group_obj *grpobj));
extern void nis_print_table __P ((const table_obj *tblobj));
extern void nis_print_link __P ((const link_obj *lnkobj));
extern void nis_print_entry __P ((const entry_obj *enobj));

/*
** nis_file (XXX INTERNAL FUNCTIONS, SHOULD NOT BE USED !!)
*/
extern directory_obj *readColdStartFile __P ((void));
extern bool_t writeColdStartFile __P ((const directory_obj *dirobj));
extern nis_object *nis_read_obj __P ((const char *obj));
extern bool_t nis_write_obj __P ((const char *file, const nis_object *obj));

/*
** nis_clone - (XXX INTERNAL FUNCTIONS, SHOULD NOT BE USED !!)
*/
extern directory_obj *nis_clone_directory __P ((const directory_obj *src,
						directory_obj *dest));
extern nis_result *nis_clone_result __P ((const nis_result *src,
					  nis_result *dest));

/* nis_free - nis_freeresult */
extern void nis_freeresult __P ((nis_result *result));
/* (XXX THE FOLLOWING ARE INTERNAL FUNCTIONS, SHOULD NOT BE USED !!) */
extern void nis_free_request __P ((ib_request *req));
extern void nis_free_directory __P ((directory_obj *dirobj));
extern void nis_free_object __P ((nis_object *obj));

/* (XXX INTERNAL FUNCTIONS, SHOULD NOT BE USED !!) */
extern nis_name __nis_default_owner __P ((char *));
extern nis_name __nis_default_group __P ((char *));
extern u_long __nis_default_ttl __P ((char *));
extern u_long __nis_default_access __P ((char *, u_long));
extern fd_result *__nis_finddirectory __P ((directory_obj *, const_nis_name));
extern void __free_fdresult __P ((fd_result *));
extern u_long __nis_hash __P ((const void *keyarg, register size_t len));

/* NIS+ cache locking */
extern int __nis_lock_cache __P ((void));
extern int __nis_unlock_cache __P ((void));

/* (XXX INTERNAL FUNCTIONS, ONLY FOR rpc.nisd AND glibc !!) */
#if defined (NIS_INTERNAL) || defined (_LIBC)

struct dir_binding
{
  CLIENT *clnt;                  /* RPC CLIENT handle */
  nis_server *server_val;        /* List of servers */
  u_int server_len;              /* # of servers */
  u_int server_used;             /* Which server we are bind in the moment ? */
  u_int current_ep;              /* Which endpoint of the server are in use? */
  u_int trys;                    /* How many server have we tried ? */
  u_int class;                   /* From which class is server_val ? */
  bool_t master_only;            /* Is only binded to the master */
  bool_t use_auth;               /* Do we use AUTH ? */
  bool_t use_udp;                /* Do we use UDP ? */
  struct sockaddr_in addr;       /* Server's IP address */
  int socket;                    /* Server's local socket */
};
typedef struct dir_binding dir_binding;

extern nis_error __nisbind_create __P ((dir_binding *, const nis_server *,
					u_int, u_long));
extern nis_error __nisbind_connect __P ((dir_binding *));
extern nis_error __nisbind_next __P ((dir_binding *));
extern void __nisbind_destroy __P ((dir_binding *));
extern nis_error __nisfind_server __P ((const_nis_name, directory_obj **));

#endif

__END_DECLS

#endif	/* __RPCSVC_NISLIB_H__ */
