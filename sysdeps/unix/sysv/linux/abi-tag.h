/* Define the GNU ABI tag for the Linux kernel we need.
   The is a 4-byte quantity in native byte order:
   the high byte is 0 to indicate Linux;
   the low three bytes are the LINUX_VERSION_CODE for the earliest
   compatible Linux kernel.  */

#define ABI_LINUX_TAG	0

#define ABI_LINUX_MAJOR	2
#define ABI_LINUX_MINOR	0
#define ABI_LINUX_PATCH	0

#define ABI_TAG ((ABI_LINUX_TAG << 24) |				      \
		 (ABI_LINUX_MAJOR << 16) |				      \
		 (ABI_LINUX_MINOR << 8) |				      \
		 (ABI_LINUX_PATCH << 0))
