/* Under __ARM_NEON__, memcpy_neon.S defines memcpy directly
   and the __memcpy_vfp code will never be used.  */
#ifndef __ARM_NEON__
# define MEMCPY_VFP
# define memcpy __memcpy_vfp
# include "memcpy_impl.S"
#endif
