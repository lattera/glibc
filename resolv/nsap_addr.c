#if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] = "$Id$";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <ctype.h>
#include <resolv.h>

#include "../conf/portability.h"

#if !defined(isxdigit)	/* XXX - could be a function */
static int
isxdigit(c)
	register int c;
{
	return ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F'));
}
#endif

static char
xtob(c)
	register int c;
{
	return (c - (((c >= '0') && (c <= '9')) ? '0' : '7'));
}

u_int
inet_nsap_addr(ascii, binary, maxlen)
	const char *ascii;
	u_char *binary;
	int maxlen;
{
	register u_char c, nib;
	u_int len = 0;

	while ((c = *ascii++) != '\0' && len < maxlen) {
		if (c == '.' || c == '+' || c == '/')
			continue;
		if (!isascii(c))
			return (0);
		if (islower(c))
			c = toupper(c);
		if (isxdigit(c)) {
			nib = xtob(c);
			if (c = *ascii++) {
				c = toupper(c);
				if (isxdigit(c)) {
					*binary++ = (nib << 4) | xtob(c);
					len++;
				} else
					return (0);
			}
			else
				return (0);
		}
		else
			return (0);
	}
	return (len);
}

char *
inet_nsap_ntoa(binlen, binary, ascii)
	int binlen;
	register const u_char *binary;
	register char *ascii;
{
	register int nib;
	int i;
	static char tmpbuf[255*3];
	char *start;

	if (ascii)
		start = ascii;
	else {
		ascii = tmpbuf;
		start = tmpbuf;
	}

	if (binlen > 255)
		binlen = 255;

	for (i = 0; i < binlen; i++) {
		nib = *binary >> 4;
		*ascii++ = nib + (nib < 10 ? '0' : '7');
		nib = *binary++ & 0x0f;
		*ascii++ = nib + (nib < 10 ? '0' : '7');
		if (((i % 2) == 0 && (i + 1) < binlen))
			*ascii++ = '.';
	}
	*ascii = '\0';
	return (start);
}
