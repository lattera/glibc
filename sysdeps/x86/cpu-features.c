/* Initialize CPU feature data.
   This file is part of the GNU C Library.
   Copyright (C) 2008-2018 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <cpuid.h>
#include <cpu-features.h>
#include <dl-hwcap.h>
#include <libc-pointer-arith.h>

#if HAVE_TUNABLES
# define TUNABLE_NAMESPACE tune
# include <unistd.h>		/* Get STDOUT_FILENO for _dl_printf.  */
# include <elf/dl-tunables.h>

extern void TUNABLE_CALLBACK (set_hwcaps) (tunable_val_t *)
  attribute_hidden;

# if CET_ENABLED
extern void TUNABLE_CALLBACK (set_x86_ibt) (tunable_val_t *)
  attribute_hidden;
extern void TUNABLE_CALLBACK (set_x86_shstk) (tunable_val_t *)
  attribute_hidden;
# endif
#endif

#if CET_ENABLED
# include <dl-cet.h>
# include <cet-tunables.h>
#endif

static void
get_extended_indices (struct cpu_features *cpu_features)
{
  unsigned int eax, ebx, ecx, edx;
  __cpuid (0x80000000, eax, ebx, ecx, edx);
  if (eax >= 0x80000001)
    __cpuid (0x80000001,
	     cpu_features->cpuid[COMMON_CPUID_INDEX_80000001].eax,
	     cpu_features->cpuid[COMMON_CPUID_INDEX_80000001].ebx,
	     cpu_features->cpuid[COMMON_CPUID_INDEX_80000001].ecx,
	     cpu_features->cpuid[COMMON_CPUID_INDEX_80000001].edx);

}

static void
get_common_indeces (struct cpu_features *cpu_features,
		    unsigned int *family, unsigned int *model,
		    unsigned int *extended_model, unsigned int *stepping)
{
  if (family)
    {
      unsigned int eax;
      __cpuid (1, eax, cpu_features->cpuid[COMMON_CPUID_INDEX_1].ebx,
	       cpu_features->cpuid[COMMON_CPUID_INDEX_1].ecx,
	       cpu_features->cpuid[COMMON_CPUID_INDEX_1].edx);
      cpu_features->cpuid[COMMON_CPUID_INDEX_1].eax = eax;
      *family = (eax >> 8) & 0x0f;
      *model = (eax >> 4) & 0x0f;
      *extended_model = (eax >> 12) & 0xf0;
      *stepping = eax & 0x0f;
      if (*family == 0x0f)
	{
	  *family += (eax >> 20) & 0xff;
	  *model += *extended_model;
	}
    }

  if (cpu_features->max_cpuid >= 7)
    __cpuid_count (7, 0,
		   cpu_features->cpuid[COMMON_CPUID_INDEX_7].eax,
		   cpu_features->cpuid[COMMON_CPUID_INDEX_7].ebx,
		   cpu_features->cpuid[COMMON_CPUID_INDEX_7].ecx,
		   cpu_features->cpuid[COMMON_CPUID_INDEX_7].edx);

  /* Can we call xgetbv?  */
  if (CPU_FEATURES_CPU_P (cpu_features, OSXSAVE))
    {
      unsigned int xcrlow;
      unsigned int xcrhigh;
      asm ("xgetbv" : "=a" (xcrlow), "=d" (xcrhigh) : "c" (0));
      /* Is YMM and XMM state usable?  */
      if ((xcrlow & (bit_YMM_state | bit_XMM_state)) ==
	  (bit_YMM_state | bit_XMM_state))
	{
	  /* Determine if AVX is usable.  */
	  if (CPU_FEATURES_CPU_P (cpu_features, AVX))
	    {
	      cpu_features->feature[index_arch_AVX_Usable]
		|= bit_arch_AVX_Usable;
	      /* The following features depend on AVX being usable.  */
	      /* Determine if AVX2 is usable.  */
	      if (CPU_FEATURES_CPU_P (cpu_features, AVX2))
	      {
		cpu_features->feature[index_arch_AVX2_Usable]
		  |= bit_arch_AVX2_Usable;

	        /* Unaligned load with 256-bit AVX registers are faster on
	           Intel/AMD processors with AVX2.  */
	        cpu_features->feature[index_arch_AVX_Fast_Unaligned_Load]
		  |= bit_arch_AVX_Fast_Unaligned_Load;
	      }
	      /* Determine if FMA is usable.  */
	      if (CPU_FEATURES_CPU_P (cpu_features, FMA))
		cpu_features->feature[index_arch_FMA_Usable]
		  |= bit_arch_FMA_Usable;
	    }

	  /* Check if OPMASK state, upper 256-bit of ZMM0-ZMM15 and
	     ZMM16-ZMM31 state are enabled.  */
	  if ((xcrlow & (bit_Opmask_state | bit_ZMM0_15_state
			 | bit_ZMM16_31_state)) ==
	      (bit_Opmask_state | bit_ZMM0_15_state | bit_ZMM16_31_state))
	    {
	      /* Determine if AVX512F is usable.  */
	      if (CPU_FEATURES_CPU_P (cpu_features, AVX512F))
		{
		  cpu_features->feature[index_arch_AVX512F_Usable]
		    |= bit_arch_AVX512F_Usable;
		  /* Determine if AVX512DQ is usable.  */
		  if (CPU_FEATURES_CPU_P (cpu_features, AVX512DQ))
		    cpu_features->feature[index_arch_AVX512DQ_Usable]
		      |= bit_arch_AVX512DQ_Usable;
		}
	    }
	}

      /* For _dl_runtime_resolve, set xsave_state_size to xsave area
	 size + integer register save size and align it to 64 bytes.  */
      if (cpu_features->max_cpuid >= 0xd)
	{
	  unsigned int eax, ebx, ecx, edx;

	  __cpuid_count (0xd, 0, eax, ebx, ecx, edx);
	  if (ebx != 0)
	    {
	      unsigned int xsave_state_full_size
		= ALIGN_UP (ebx + STATE_SAVE_OFFSET, 64);

	      cpu_features->xsave_state_size
		= xsave_state_full_size;
	      cpu_features->xsave_state_full_size
		= xsave_state_full_size;

	      __cpuid_count (0xd, 1, eax, ebx, ecx, edx);

	      /* Check if XSAVEC is available.  */
	      if ((eax & (1 << 1)) != 0)
		{
		  unsigned int xstate_comp_offsets[32];
		  unsigned int xstate_comp_sizes[32];
		  unsigned int i;

		  xstate_comp_offsets[0] = 0;
		  xstate_comp_offsets[1] = 160;
		  xstate_comp_offsets[2] = 576;
		  xstate_comp_sizes[0] = 160;
		  xstate_comp_sizes[1] = 256;

		  for (i = 2; i < 32; i++)
		    {
		      if ((STATE_SAVE_MASK & (1 << i)) != 0)
			{
			  __cpuid_count (0xd, i, eax, ebx, ecx, edx);
			  xstate_comp_sizes[i] = eax;
			}
		      else
			{
			  ecx = 0;
			  xstate_comp_sizes[i] = 0;
			}

		      if (i > 2)
			{
			  xstate_comp_offsets[i]
			    = (xstate_comp_offsets[i - 1]
			       + xstate_comp_sizes[i -1]);
			  if ((ecx & (1 << 1)) != 0)
			    xstate_comp_offsets[i]
			      = ALIGN_UP (xstate_comp_offsets[i], 64);
			}
		    }

		  /* Use XSAVEC.  */
		  unsigned int size
		    = xstate_comp_offsets[31] + xstate_comp_sizes[31];
		  if (size)
		    {
		      cpu_features->xsave_state_size
			= ALIGN_UP (size + STATE_SAVE_OFFSET, 64);
		      cpu_features->feature[index_arch_XSAVEC_Usable]
			|= bit_arch_XSAVEC_Usable;
		    }
		}
	    }
	}
    }
}

static inline void
init_cpu_features (struct cpu_features *cpu_features)
{
  unsigned int ebx, ecx, edx;
  unsigned int family = 0;
  unsigned int model = 0;
  enum cpu_features_kind kind;

#if !HAS_CPUID
  if (__get_cpuid_max (0, 0) == 0)
    {
      kind = arch_kind_other;
      goto no_cpuid;
    }
#endif

  __cpuid (0, cpu_features->max_cpuid, ebx, ecx, edx);

  /* This spells out "GenuineIntel".  */
  if (ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69)
    {
      unsigned int extended_model, stepping;

      kind = arch_kind_intel;

      get_common_indeces (cpu_features, &family, &model, &extended_model,
			  &stepping);

      get_extended_indices (cpu_features);

      if (family == 0x06)
	{
	  model += extended_model;
	  switch (model)
	    {
	    case 0x1c:
	    case 0x26:
	      /* BSF is slow on Atom.  */
	      cpu_features->feature[index_arch_Slow_BSF]
		|= bit_arch_Slow_BSF;
	      break;

	    case 0x57:
	      /* Knights Landing.  Enable Silvermont optimizations.  */

	    case 0x5c:
	    case 0x5f:
	      /* Unaligned load versions are faster than SSSE3
		 on Goldmont.  */

	    case 0x4c:
	      /* Airmont is a die shrink of Silvermont.  */

	    case 0x37:
	    case 0x4a:
	    case 0x4d:
	    case 0x5a:
	    case 0x5d:
	      /* Unaligned load versions are faster than SSSE3
		 on Silvermont.  */
#if index_arch_Fast_Unaligned_Load != index_arch_Prefer_PMINUB_for_stringop
# error index_arch_Fast_Unaligned_Load != index_arch_Prefer_PMINUB_for_stringop
#endif
#if index_arch_Fast_Unaligned_Load != index_arch_Slow_SSE4_2
# error index_arch_Fast_Unaligned_Load != index_arch_Slow_SSE4_2
#endif
#if index_arch_Fast_Unaligned_Load != index_arch_Fast_Unaligned_Copy
# error index_arch_Fast_Unaligned_Load != index_arch_Fast_Unaligned_Copy
#endif
	      cpu_features->feature[index_arch_Fast_Unaligned_Load]
		|= (bit_arch_Fast_Unaligned_Load
		    | bit_arch_Fast_Unaligned_Copy
		    | bit_arch_Prefer_PMINUB_for_stringop
		    | bit_arch_Slow_SSE4_2);
	      break;

	    default:
	      /* Unknown family 0x06 processors.  Assuming this is one
		 of Core i3/i5/i7 processors if AVX is available.  */
	      if (!CPU_FEATURES_CPU_P (cpu_features, AVX))
		break;

	    case 0x1a:
	    case 0x1e:
	    case 0x1f:
	    case 0x25:
	    case 0x2c:
	    case 0x2e:
	    case 0x2f:
	      /* Rep string instructions, unaligned load, unaligned copy,
		 and pminub are fast on Intel Core i3, i5 and i7.  */
#if index_arch_Fast_Rep_String != index_arch_Fast_Unaligned_Load
# error index_arch_Fast_Rep_String != index_arch_Fast_Unaligned_Load
#endif
#if index_arch_Fast_Rep_String != index_arch_Prefer_PMINUB_for_stringop
# error index_arch_Fast_Rep_String != index_arch_Prefer_PMINUB_for_stringop
#endif
#if index_arch_Fast_Rep_String != index_arch_Fast_Unaligned_Copy
# error index_arch_Fast_Rep_String != index_arch_Fast_Unaligned_Copy
#endif
	      cpu_features->feature[index_arch_Fast_Rep_String]
		|= (bit_arch_Fast_Rep_String
		    | bit_arch_Fast_Unaligned_Load
		    | bit_arch_Fast_Unaligned_Copy
		    | bit_arch_Prefer_PMINUB_for_stringop);
	      break;

	    case 0x3f:
	      /* Xeon E7 v3 with stepping >= 4 has working TSX.  */
	      if (stepping >= 4)
		break;
	    case 0x3c:
	    case 0x45:
	    case 0x46:
	      /* Disable Intel TSX on Haswell processors (except Xeon E7 v3
		 with stepping >= 4) to avoid TSX on kernels that weren't
		 updated with the latest microcode package (which disables
		 broken feature by default).  */
	      cpu_features->cpuid[index_cpu_RTM].reg_RTM &= ~bit_cpu_RTM;
	      break;
	    }
	}


      /* Since AVX512ER is unique to Xeon Phi, set Prefer_No_VZEROUPPER
         if AVX512ER is available.  Don't use AVX512 to avoid lower CPU
	 frequency if AVX512ER isn't available.  */
      if (CPU_FEATURES_CPU_P (cpu_features, AVX512ER))
	cpu_features->feature[index_arch_Prefer_No_VZEROUPPER]
	  |= bit_arch_Prefer_No_VZEROUPPER;
      else
	cpu_features->feature[index_arch_Prefer_No_AVX512]
	  |= bit_arch_Prefer_No_AVX512;
    }
  /* This spells out "AuthenticAMD".  */
  else if (ebx == 0x68747541 && ecx == 0x444d4163 && edx == 0x69746e65)
    {
      unsigned int extended_model, stepping;

      kind = arch_kind_amd;

      get_common_indeces (cpu_features, &family, &model, &extended_model,
			  &stepping);

      get_extended_indices (cpu_features);

      ecx = cpu_features->cpuid[COMMON_CPUID_INDEX_1].ecx;

      if (HAS_ARCH_FEATURE (AVX_Usable))
	{
	  /* Since the FMA4 bit is in COMMON_CPUID_INDEX_80000001 and
	     FMA4 requires AVX, determine if FMA4 is usable here.  */
	  if (CPU_FEATURES_CPU_P (cpu_features, FMA4))
	    cpu_features->feature[index_arch_FMA4_Usable]
	      |= bit_arch_FMA4_Usable;
	}

      if (family == 0x15)
	{
#if index_arch_Fast_Unaligned_Load != index_arch_Fast_Copy_Backward
# error index_arch_Fast_Unaligned_Load != index_arch_Fast_Copy_Backward
#endif
	  /* "Excavator"   */
	  if (model >= 0x60 && model <= 0x7f)
	  {
	    cpu_features->feature[index_arch_Fast_Unaligned_Load]
	      |= (bit_arch_Fast_Unaligned_Load
		  | bit_arch_Fast_Copy_Backward);

	    /* Unaligned AVX loads are slower.*/
	    cpu_features->feature[index_arch_AVX_Fast_Unaligned_Load]
		  &= ~bit_arch_AVX_Fast_Unaligned_Load;
	  }
	}
    }
  else
    {
      kind = arch_kind_other;
      get_common_indeces (cpu_features, NULL, NULL, NULL, NULL);
    }

  /* Support i586 if CX8 is available.  */
  if (CPU_FEATURES_CPU_P (cpu_features, CX8))
    cpu_features->feature[index_arch_I586] |= bit_arch_I586;

  /* Support i686 if CMOV is available.  */
  if (CPU_FEATURES_CPU_P (cpu_features, CMOV))
    cpu_features->feature[index_arch_I686] |= bit_arch_I686;

#if !HAS_CPUID
no_cpuid:
#endif

  cpu_features->family = family;
  cpu_features->model = model;
  cpu_features->kind = kind;

#if HAVE_TUNABLES
  TUNABLE_GET (hwcaps, tunable_val_t *, TUNABLE_CALLBACK (set_hwcaps));
  cpu_features->non_temporal_threshold
    = TUNABLE_GET (x86_non_temporal_threshold, long int, NULL);
  cpu_features->data_cache_size
    = TUNABLE_GET (x86_data_cache_size, long int, NULL);
  cpu_features->shared_cache_size
    = TUNABLE_GET (x86_shared_cache_size, long int, NULL);
#endif

  /* Reuse dl_platform, dl_hwcap and dl_hwcap_mask for x86.  */
#if !HAVE_TUNABLES && defined SHARED
  /* The glibc.tune.hwcap_mask tunable is initialized already, so no need to do
     this.  */
  GLRO(dl_hwcap_mask) = HWCAP_IMPORTANT;
#endif

#ifdef __x86_64__
  GLRO(dl_hwcap) = HWCAP_X86_64;
  if (cpu_features->kind == arch_kind_intel)
    {
      const char *platform = NULL;

      if (CPU_FEATURES_ARCH_P (cpu_features, AVX512F_Usable)
	  && CPU_FEATURES_CPU_P (cpu_features, AVX512CD))
	{
	  if (CPU_FEATURES_CPU_P (cpu_features, AVX512ER))
	    {
	      if (CPU_FEATURES_CPU_P (cpu_features, AVX512PF))
		platform = "xeon_phi";
	    }
	  else
	    {
	      if (CPU_FEATURES_CPU_P (cpu_features, AVX512BW)
		  && CPU_FEATURES_CPU_P (cpu_features, AVX512DQ)
		  && CPU_FEATURES_CPU_P (cpu_features, AVX512VL))
		GLRO(dl_hwcap) |= HWCAP_X86_AVX512_1;
	    }
	}

      if (platform == NULL
	  && CPU_FEATURES_ARCH_P (cpu_features, AVX2_Usable)
	  && CPU_FEATURES_ARCH_P (cpu_features, FMA_Usable)
	  && CPU_FEATURES_CPU_P (cpu_features, BMI1)
	  && CPU_FEATURES_CPU_P (cpu_features, BMI2)
	  && CPU_FEATURES_CPU_P (cpu_features, LZCNT)
	  && CPU_FEATURES_CPU_P (cpu_features, MOVBE)
	  && CPU_FEATURES_CPU_P (cpu_features, POPCNT))
	platform = "haswell";

      if (platform != NULL)
	GLRO(dl_platform) = platform;
    }
#else
  GLRO(dl_hwcap) = 0;
  if (CPU_FEATURES_CPU_P (cpu_features, SSE2))
    GLRO(dl_hwcap) |= HWCAP_X86_SSE2;

  if (CPU_FEATURES_ARCH_P (cpu_features, I686))
    GLRO(dl_platform) = "i686";
  else if (CPU_FEATURES_ARCH_P (cpu_features, I586))
    GLRO(dl_platform) = "i586";
#endif

#if CET_ENABLED
# if HAVE_TUNABLES
  TUNABLE_GET (x86_ibt, tunable_val_t *,
	       TUNABLE_CALLBACK (set_x86_ibt));
  TUNABLE_GET (x86_shstk, tunable_val_t *,
	       TUNABLE_CALLBACK (set_x86_shstk));
# endif

  /* Check CET status.  */
  unsigned int cet_status = get_cet_status ();

  if (cet_status)
    {
      GL(dl_x86_feature_1)[0] = cet_status;

# ifndef SHARED
      /* Check if IBT and SHSTK are enabled by kernel.  */
      if ((cet_status & GNU_PROPERTY_X86_FEATURE_1_IBT)
	  || (cet_status & GNU_PROPERTY_X86_FEATURE_1_SHSTK))
	{
	  /* Disable IBT and/or SHSTK if they are enabled by kernel, but
	     disabled by environment variable:

	     GLIBC_TUNABLES=glibc.tune.hwcaps=-IBT,-SHSTK
	   */
	  unsigned int cet_feature = 0;
	  if (!HAS_CPU_FEATURE (IBT))
	    cet_feature |= GNU_PROPERTY_X86_FEATURE_1_IBT;
	  if (!HAS_CPU_FEATURE (SHSTK))
	    cet_feature |= GNU_PROPERTY_X86_FEATURE_1_SHSTK;

	  if (cet_feature)
	    {
	      int res = dl_cet_disable_cet (cet_feature);

	      /* Clear the disabled bits in dl_x86_feature_1.  */
	      if (res == 0)
		GL(dl_x86_feature_1)[0] &= ~cet_feature;
	    }

	  /* Lock CET if IBT or SHSTK is enabled in executable.  Don't
	     lock CET if SHSTK is enabled permissively.  */
	  if (((GL(dl_x86_feature_1)[1] >> CET_MAX)
	       & ((1 << CET_MAX) - 1))
	       != CET_PERMISSIVE)
	    dl_cet_lock_cet ();
	}
# endif
    }
#endif
}
