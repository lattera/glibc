/* Copyright (c) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1998.

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

#ifndef __NIS_XDR_H
#define __NIS_XDR_H 1

#include <features.h>

__BEGIN_DECLS

extern  bool_t _xdr_nis_attr __P ((XDR *, nis_attr*));
extern  bool_t _xdr_nis_name __P ((XDR *, nis_name*));
extern  bool_t _xdr_nis_server __P ((XDR *, nis_server*));
extern  bool_t _xdr_directory_obj __P ((XDR *, directory_obj*));
extern  bool_t _xdr_nis_object __P ((XDR *, nis_object*));
extern  bool_t _xdr_nis_error __P ((XDR *, nis_error*));
extern  bool_t _xdr_nis_result __P ((XDR *, nis_result*));
extern  bool_t _xdr_ns_request __P ((XDR *, ns_request*));
extern  bool_t _xdr_ib_request __P ((XDR *, ib_request*));
extern  bool_t _xdr_ping_args __P ((XDR *, ping_args*));
extern  bool_t _xdr_cp_result __P ((XDR *, cp_result*));
extern  bool_t _xdr_nis_tag __P ((XDR *, nis_tag*));
extern  bool_t _xdr_nis_taglist __P ((XDR *, nis_taglist*));
extern  bool_t _xdr_fd_args __P ((XDR *, fd_args*));
extern  bool_t _xdr_fd_result __P ((XDR *, fd_result*));

__END_DECLS

#endif
