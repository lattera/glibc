/* Copyright (C) 1997 Free Software Foundation, Inc.
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

/*
** nis_names
*/
extern nis_result *nis_lookup __P ((__const nis_name, const u_long));
extern nis_result *nis_add __P ((__const nis_name, const nis_object *));
extern nis_result *nis_remove __P ((__const nis_name, const nis_object *));
extern nis_result *nis_modify __P ((__const nis_name, const nis_object *));
/*
** nis_table
*/
extern nis_result *nis_list __P ((__const nis_name, const u_long,
				  int (*)(__const nis_name,
					  __const nis_object *,
					  __const void *), __const void *));
extern nis_result *nis_add_entry __P ((__const nis_name, __const nis_object *,
				       __const u_long));
extern nis_result *nis_modify_entry __P ((__const nis_name,
					  __const nis_object *,
					  __const u_long));
extern nis_result *nis_remove_entry __P ((__const nis_name,
					  __const nis_object *,
					  __const u_long));
extern nis_result *nis_first_entry __P ((__const nis_name));
extern nis_result *nis_next_entry __P ((__const nis_name, __const netobj *));
/*
** nis_server
*/
extern nis_error nis_mkdir __P ((__const nis_name, __const nis_server *));
extern nis_error nis_rmdir __P ((__const nis_name, __const nis_server *));
extern nis_error nis_servstate __P ((__const nis_server *, __const nis_tag *,
				     __const int, nis_tag **));
extern nis_error nis_stats __P ((__const nis_server *, __const nis_tag *,
				 __const int, nis_tag **));
extern void nis_freetags __P ((nis_tag *, __const int));
extern nis_server **nis_getservlist __P ((__const nis_name));
extern void nis_freeservlist __P ((nis_server **));
/*
** nis_subr
*/
extern nis_name nis_leaf_of __P ((__const nis_name));
extern nis_name nis_leaf_of_r __P ((__const nis_name, char *, size_t));
extern nis_name nis_name_of __P ((__const nis_name));
extern nis_name nis_name_of_r __P ((__const nis_name, char *, size_t));
extern nis_name nis_domain_of __P ((__const nis_name));
extern nis_name nis_domain_of_r __P ((__const nis_name, char *, size_t));
extern nis_name *nis_getnames __P ((__const nis_name));
extern void nis_freenames __P ((nis_name *));
extern name_pos nis_dir_cmp __P ((nis_name, nis_name));
extern nis_object *nis_clone_object __P ((__const nis_object *, nis_object *));
extern void nis_destroy_object __P ((nis_object *));
extern void nis_print_object __P ((__const nis_object *));
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
extern const char *nis_sperrno __P ((__const nis_error));
extern void nis_perror __P ((__const nis_error, __const char *));
extern void nis_lerror __P ((__const nis_error, __const char *));
extern char *nis_sperror __P ((__const nis_error, __const char *));
extern char *nis_sperror_r __P ((__const nis_error, __const char *,
				 char *, size_t));
/*
** nis_groups
*/
extern bool_t nis_ismember __P ((__const nis_name, __const nis_name));
extern nis_error nis_addmember __P ((__const nis_name, __const nis_name));
extern nis_error nis_removemember __P ((__const nis_name, __const nis_name));
extern nis_error nis_creategroup __P ((__const nis_name, __const u_long));
extern nis_error nis_destroygroup __P ((__const nis_name));
extern void nis_print_group_entry __P ((__const nis_name));
extern nis_error nis_verifygroup __P ((__const nis_name));
/*
** nis_ping
*/
extern void nis_ping __P ((__const nis_name, __const u_long,
			   __const nis_object *));
extern nis_result *nis_checkpoint __P ((__const nis_name));

/*
** nis_print (XXX INTERNAL FUNCTIONS, SHOULD NOT BE USED !!)
*/
extern void nis_print_result __P ((__const nis_result *));
extern void nis_print_rights __P ((__const u_long));
extern void nis_print_directory __P ((__const directory_obj *));
extern void nis_print_group __P ((__const group_obj *));
extern void nis_print_table __P ((__const table_obj *));
extern void nis_print_link __P ((__const link_obj *));
extern void nis_print_entry __P ((__const entry_obj *));
/*
** nis_file (XXX INTERNAL FUNCTIONS, SHOULD NOT BE USED !!)
*/
extern directory_obj *readColdStartFile __P ((void));
extern bool_t writeColdStartFile __P ((__const directory_obj *));
extern nis_object *nis_read_obj __P ((__const char *));
extern bool_t nis_write_obj __P ((__const char *, __const nis_object *));
/*
** nis_clone - (XXX INTERNAL FUNCTIONS, SHOULD NOT BE USED !!)
*/

extern directory_obj *nis_clone_directory __P ((__const directory_obj *,
						directory_obj *));
extern group_obj *nis_clone_group __P ((__const group_obj *, group_obj *));
extern table_obj *nis_clone_table __P ((__const table_obj *, table_obj *));
extern entry_obj *nis_clone_entry __P ((__const entry_obj *, entry_obj *));
extern link_obj *nis_clone_link __P ((__const link_obj *, link_obj *));
extern objdata *nis_clone_objdata __P ((__const objdata *, objdata *));
extern nis_result *nis_clone_result __P ((__const nis_result *, nis_result *));
/*
** nis_free - nis_freeresult
*/
extern void nis_freeresult __P ((nis_result *));
/* (XXX INTERNAL FUNCTIONS, SHOULD NOT BE USED !!) */
extern void nis_free_attr __P ((nis_attr *));
extern void nis_free_request __P ((ib_request *));
extern void nis_free_endpoints __P ((endpoint *, unsigned int));
extern void nis_free_servers __P ((nis_server *, unsigned int));
extern void nis_free_directory __P ((directory_obj *));
extern void nis_free_group __P ((group_obj *));
extern void nis_free_table __P ((table_obj *));
extern void nis_free_entry __P ((entry_obj *));
extern void nis_free_link __P ((link_obj *));
extern void nis_free_object __P ((nis_object *));

/* This is the SUN definition, but I don't know for what we need
   the directory_obj parameter */
/* extern fd_result *nis_finddirectory __P ((directory_obj *, nis_name)); */
extern fd_result *__nis_finddirectory __P ((nis_name));
extern int __start_clock(int);
extern u_long __stop_clock(int);

__END_DECLS

#endif	/* __RPCSVC_NISLIB_H__ */
