#define lio_listio64 __renamed_lio_listio64

#include "../../../../pthread/lio_listio.c"

#undef lio_listio64

#if SHLIB_COMPAT (librt, GLIBC_2_1, GLIBC_2_4)
strong_alias (__lio_listio_21, __lio_listio64_21)
compat_symbol (librt, __lio_listio64_21, lio_listio64, GLIBC_2_1);
#endif

strong_alias (__lio_listio_item_notify, __lio_listio64_item_notify)
versioned_symbol (librt, __lio_listio64_item_notify, lio_listio64, GLIBC_2_4);
