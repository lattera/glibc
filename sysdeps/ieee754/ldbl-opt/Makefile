# The`long double' type is a distinct type we support if
# -mlong-double-128 option is used (or when it becomes a default
# when -mlong-double-64 is not used).
long-double-fcts = yes
ifeq (,$(filter -mlong-double-128,$(sysdep-CFLAGS)))
sysdep-CFLAGS += -mlong-double-128
endif

ifeq ($(subdir),math)
libm-routines += s_nexttowardfd
routines += math_ldbl_opt nldbl-compat

extra-libs += libnldbl
libnldbl-calls = asprintf dprintf fprintf fscanf fwprintf fwscanf iovfscanf \
		 obstack_printf obstack_vprintf printf scanf snprintf \
		 sprintf sscanf swprintf swscanf vasprintf vdprintf vfprintf \
		 vfscanf vfwprintf vfwscanf vprintf vscanf vsnprintf \
		 vsprintf vsscanf vswprintf vswscanf vwprintf vwscanf \
		 wprintf wscanf printf_fp printf_size \
		 fprintf_chk fwprintf_chk printf_chk snprintf_chk sprintf_chk \
		 swprintf_chk vfprintf_chk vfwprintf_chk vprintf_chk \
		 vsnprintf_chk vsprintf_chk vswprintf_chk vwprintf_chk \
		 wprintf_chk asprintf_chk vasprintf_chk dprintf_chk \
		 vdprintf_chk obstack_printf_chk obstack_vprintf_chk \
		 syslog syslog_chk vsyslog vsyslog_chk \
		 strfmon strfmon_l \
		 strtold strtold_l strtoldint wcstold wcstold_l wcstoldint \
		 qecvt qfcvt qgcvt qecvt_r qfcvt_r \
		 isinf isnan finite signbit scalb log2 lgamma_r ceil \
		 significand acos asin atan atan2 cos sin tan cosh sinh \
		 tanh acosh asinh atanh exp log log10 exp10 pow10 expm1 \
		 log1p logb exp2 sqrt cbrt fabs floor j0 j1 y0 y1 erf erfc \
		 lgamma tgamma gamma rint nearbyint round trunc \
		 copysign fdim fmax fmin nextafter pow hypot fmod drem \
		 remainder ldexp scalbn frexp modf scalbln fma nan sincos \
		 jn yn ilogb remquo lrint lround llrint llround nexttowardf \
		 nexttoward conj cacos cacosh casin catan catanh ccos ccosh \
		 casinh cexp clog cproj csin csinh csqrt ctan ctanh cpow \
		 cabs carg cimag creal clog10 \
		 isoc99_scanf isoc99_fscanf isoc99_sscanf \
		 isoc99_vscanf isoc99_vfscanf isoc99_vsscanf \
		 isoc99_wscanf isoc99_fwscanf isoc99_swscanf \
		 isoc99_vwscanf isoc99_vfwscanf isoc99_vswscanf
libnldbl-routines = $(libnldbl-calls:%=nldbl-%)
libnldbl-inhibit-o = $(object-suffixes)
libnldbl-static-only-routines = $(libnldbl-routines)
extra-objs += $(addsuffix .oS, $(libnldbl-routines))

endif
