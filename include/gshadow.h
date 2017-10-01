#ifndef _GSHADOW_H
#include <gshadow/gshadow.h>

# ifndef _ISOMAC

extern int __fgetsgent_r (FILE *stream, struct sgrp *resbuf, char *buffer,
			  size_t buflen, struct sgrp **result)
     attribute_hidden;
extern int __sgetsgent_r (const char *string, struct sgrp *resbuf,
			  char *buffer, size_t buflen, struct sgrp **result)
     attribute_hidden;

struct parser_data;
extern int _nss_files_parse_sgent (char *line, struct sgrp *result,
                                   struct parser_data *data,
                                   size_t datalen, int *errnop);
libc_hidden_proto (_nss_files_parse_sgent)

# endif /* !_ISOMAC */
#endif
