/* This file uses the getaddrinfo code but it compiles it without NSCD
   support.  We just need a few symbol renames.  */
#define __getservbyname_r getservbyname_r
#define __inet_aton inet_aton
#define __getsockname getsockname
#define __socket socket
#define __recvmsg recvmsg
#define __bind bind
#define __sendto sendto
#define __strchrnul strchrnul

#include <getaddrinfo.c>

/* Support code.  */
#include <check_pf.c>
#ifdef HAVE_LIBIDN
# include <libidn/idn-stub.c>
#endif

/* Some variables normally defined in libc.  */
service_user *__nss_hosts_database;

#if defined NEED_NETLINK && __ASSUME_NETLINK_SUPPORT == 0
int __no_netlink_support attribute_hidden;
#endif
