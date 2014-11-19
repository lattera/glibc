/* Valid values for the IN_MODULE macro, which is defined for each source file
   during compilation to indicate which module it is to be built into.

   TODO: This file should eventually be auto-generated.  */
#define MODULE_libc		1
#define MODULE_libpthread	2
#define MODULE_rtld		3
#define MODULE_libdl		4
#define MODULE_libm		5
#define MODULE_iconvprogs	6
#define MODULE_iconvdata	7
#define MODULE_lddlibc4		8
#define MODULE_locale_programs	9
#define MODULE_memusagestat	10
#define MODULE_libutil		12
#define MODULE_libBrokenLocale	13
#define MODULE_libmemusage	15
#define MODULE_libresolv	16
#define MODULE_libnss_db	17
#define MODULE_libnss_files	18
#define	MODULE_libnss_dns	19
#define MODULE_libnss_compat	20
#define MODULE_libnss_hesiod	21
#define MODULE_libnss_nis	22
#define MODULE_libnss_nisplus	23
#define MODULE_libanl		24
#define MODULE_librt		25
#define MODULE_libSegFault	26
#define MODULE_libthread_db	27
#define MODULE_libcidn		28
#define MODULE_libcrypt		29
#define MODULE_libnsl		30
#define MODULE_libpcprofile	31
#define MODULE_librpcsvc	32
#define MODULE_nscd		33
#define MODULE_ldconfig 	34
#define MODULE_libnldbl 	35

/* Catch-all for test modules and other binaries.  */
#define MODULE_nonlib		98
#define MODULE_extramodules	99
