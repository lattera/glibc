/* Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */


/*

 int __loadx(flag, module, arg1, arg2, arg3)

 The __loadx() is a call to ld_loadutil() kernel function, which 
 does the real work. Note ld_loadutil() is not exported an cannot be
 called directly from user space.

 void *ld_loadutil() call is a utility function used for loader extensions
 supporting run-time linking and dl*() functions.

 void *   - will return the modules entry point if it succeds of NULL
                on failure.

 int flag - the flag field performas a dual role: the top 8 bits specify
            the work for __loadx() to perform, the bottom 8 bits are
            used to pass flags to the work routines, all other bits are
            reserved.

*/

#define DL_LOAD       0x1000000 /* __loadx(flag,buf, buf_len, filename, libr_path) */
#define DL_POSTLOADQ  0x2000000 /* __loadx(flag,buf, buf_len, module_handle) */
#define DL_EXECQ      0x3000000 /* __loadx(flag,buf, buf_len) */
#define DL_EXITQ      0x4000000 /* __loadx(flag,buf, buf_len) */
#define DL_PREUNLOADQ 0x5000000 /* __loadx(flag,buf, buf_len, module_handle) */
#define DL_INIT       0x6000000 /* __loadx(flag,NULL) */
#define DL_GETSYM     0x7000000 /* __loadx(flag,symbol, index, modules_data_origin) */
#define DL_SETDEPEND  0x8000000 /* __loadx(flag,import_data_org, import_index, */
                                /*              export_data_org, export_index) */
#define DL_DELDEPEND  0x9000000 /* __loadx(flag,import_data_org, import_index, */
                                /*              export_data_org, export_index) */
#define DL_GLOBALSYM  0xA000000 /* __loadx(flag,symbol_name, ptr_to_rec_index, */
                                /*                        ptr_to_rec_data_org) */
#define DL_UNIX_SYSCALL 0xB000000 /* __loadx(flag,syscall_symbol_name) */

#define DL_FUNCTION_MASK 0xFF000000
#define DL_SRCHDEPENDS   0x00100000
#define DL_SRCHMODULE    0x00080000
#define DL_SRCHLOADLIST  0x00040000
#define DL_LOAD_LDX1     0x00040000
#define DL_LOAD_RTL      0x00020000
#define DL_HASHSTRING    0x00020000
#define DL_INFO_OK       0x00010000
#define DL_LOAD_DLINFO   0x00010000
#define DL_UNLOADED      0x00020000

typedef union _dl_info
{
  struct {
           uint      _xflags;   /* flag bits in the array         */
           uint      _size;     /* size of this structure         */
           uint      _arraylen; /* number of following elements   */
         } _dl_stat;
  struct {
           caddr_t   _textorg;  /* start of loaded program image  */
           caddr_t   _dataorg;  /* start of data instance         */
           uint      _datasize; /* size of data instance          */
           ushort    _index;    /* index of this le in la_dynlist */
           ushort    _mflags;   /* info about module from load()  */
         } _dl_array;
} DL_INFO;

#define dlinfo_xflags   _dl_stat._xflags
#define dlinfo_arraylen _dl_stat._arraylen
#define dlinfo_size     _dl_stat._size

#define dlinfo_textorg  _dl_array._textorg
#define dlinfo_datasize _dl_array._datasize
#define dlinfo_dataorg  _dl_array._dataorg
#define dlinfo_index    _dl_array._index
#define dlinfo_flags    _dl_array._mflags

#define DL_HAS_RTINIT  0x1  /* indicates the module __rtinit symbols */
#define DL_IS_NEW      0x2  /* indicates that the module is newly loaded */

struct _xArgs
{
   char    *libpath;
   DL_INFO *info;
   uint     infosize;
};

/* Shared Object DATA used for dl-open,dl-sym & dl-close support */
typedef struct
{
  void   *handle;         /* handle for __loadx    */
  uint    type;           /* type of __loadx flag  */
  ushort  index;          /* dlinfo_index          */
  caddr_t dataorg;        /* dlinfo_dataorg        */
} DL_SODATA;

