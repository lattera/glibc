#include <sparc-ifunc.h>

#define sha512_process_block sha512_process_block_generic
extern void sha512_process_block_generic (const void *buffer, size_t len,
					  struct sha512_ctx *ctx);

#include <crypt/sha512-block.c>

#undef sha512_process_block

extern void __sha512_process_block_crop (const void *buffer, size_t len,
					 struct sha512_ctx *ctx);

static bool cpu_supports_sha512(int hwcap)
{
  unsigned long cfr;

  if (!(hwcap & HWCAP_SPARC_CRYPTO))
    return false;

  __asm__ ("rd %%asr26, %0" : "=r" (cfr));
  if (cfr & (1 << 6))
    return true;

  return false;
}

extern void sha512_process_block (const void *buffer, size_t len,
				  struct sha512_ctx *ctx);
sparc_libc_ifunc(sha512_process_block, cpu_supports_sha512(hwcap) ? __sha512_process_block_crop : sha512_process_block_generic);
