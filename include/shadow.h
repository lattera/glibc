#ifndef _SHADOW_H
#include <shadow/shadow.h>

/* Now define the internal interfaces.  */
extern int __getspent_r (struct spwd *__result_buf, char *__buffer,
			 size_t __buflen, struct spwd **__result)
     attribute_hidden;
extern int __old_getspent_r (struct spwd *__result_buf, char *__buffer,
			     size_t __buflen, struct spwd **__result);
extern int __getspnam_r (__const char *__name, struct spwd *__result_buf,
			 char *__buffer, size_t __buflen,
			 struct spwd **__result);
extern int __old_getspnam_r (__const char *__name, struct spwd *__result_buf,
			     char *__buffer, size_t __buflen,
			     struct spwd **__result);
extern int __sgetspent_r (__const char *__string,
			  struct spwd *__result_buf, char *__buffer,
			  size_t __buflen, struct spwd **__result);
extern int __fgetspent_r (FILE *__stream, struct spwd *__result_buf,
			  char *__buffer, size_t __buflen,
			  struct spwd **__result);
extern int __lckpwdf (void);
extern int __ulckpwdf (void);

struct parser_data;
extern int _nss_files_parse_spent (char *line, struct spwd *result,
				   struct parser_data *data,
				   size_t datalen, int *errnop);
libc_hidden_proto (_nss_files_parse_spent)

#define DECLARE_NSS_PROTOTYPES(service)					\
extern enum nss_status _nss_ ## service ## _setspent (int);		\
extern enum nss_status _nss_ ## service ## _endspent (void);		\
extern enum nss_status _nss_ ## service ## _getspent_r			\
                       (struct spwd *pwd, char *buffer, size_t buflen,	\
			int *errnop);					\
extern enum nss_status _nss_ ## service ## _getspnam_r			\
                       (const char *name, struct spwd *pwd,		\
			char *buffer, size_t buflen, int *errnop);

DECLARE_NSS_PROTOTYPES (compat)
DECLARE_NSS_PROTOTYPES (files)
DECLARE_NSS_PROTOTYPES (hesiod)
DECLARE_NSS_PROTOTYPES (nis)
DECLARE_NSS_PROTOTYPES (nisplus)

#undef DECLARE_NSS_PROTOTYPES


#endif
