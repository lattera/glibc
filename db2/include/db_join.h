/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1998
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)db_join.h	10.2 (Sleepycat) 10/4/98
 */

#ifndef _DB_JOIN_H
#define _DB_JOIN_H
/*
 * Joins use a join cursor that is similar to a regular DB cursor except
 * that it only supports c_get and c_close functionality.  Also, it does
 * not support the full range of flags for get.
 */
typedef struct __join_cursor {
	u_int32_t j_init;		/* Set when cursor is initialized. */
	DBC 	**j_curslist;		/* Array of cursors in the join. */
	DB	 *j_primary;		/* Primary dbp. */
	DBT	  j_key;		/* Used to do lookups. */
} JOIN_CURSOR;
#endif
