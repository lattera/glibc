#ifndef _NSS_H
#include <nss/nss.h>

# ifndef _ISOMAC

#define NSS_INVALID_FIELD_CHARACTERS ":\n"
extern const char __nss_invalid_field_characters[] attribute_hidden;

_Bool __nss_valid_field (const char *value) attribute_hidden;
_Bool __nss_valid_list_field (char **list)  attribute_hidden;
const char *__nss_rewrite_field (const char *value, char **to_be_freed)
  attribute_hidden;

# endif /* !_ISOMAC */
#endif /* _NSS_H */
