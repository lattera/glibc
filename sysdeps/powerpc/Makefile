ifeq ($(subdir),math)
libm-support += fenv_const fe_nomask t_sqrt

# These routines have not been tested, so it's probable they don't work;
# and they don't provide the complete list of FP routines.  So there's
# no point in compiling them.
#sysdep_routines += q_feq q_fne q_utoq q_dtoq q_itoq q_stoq q_neg q_ulltoq \
#	           q_lltoq q_qtou q_qtoi q_qtoull q_qtoll q_qtos

tests += test-arith test-arithf
LDLIBS-test-arith = libm
LDLIBS-test-arithf = libm
endif
