#define ABI_HURD_TAG	1

#define ABI_HURD_MAJOR	0
#define ABI_HURD_MINOR	0
#define ABI_HURD_PATCH	0

/* Don't use `|' in this expression, it is a comment character in the
   assembler.  */
#define ABI_TAG ((ABI_HURD_TAG << 24) +					      \
		 (ABI_HURD_MAJOR << 16) +				      \
		 (ABI_HURD_MINOR << 8) +				      \
		 (ABI_HURD_PATCH << 0))
