# Script used in producing headers of assembly constants from C expressions.
# The input to this script looks like:
#	#cpp-directive ...
#	NAME1
#	NAME2 expression ...
# The output of this script is C code to be run through gcc -S and then
# massaged to extract the integer constant values of the given C expressions.
# A line giving just a name implies an expression consisting of just that name.

BEGIN { started = 0 }

# cpp directives go straight through.
/^#/ { print; next }

NF >= 1 && !started {
  if (test) {
    print "\n#include <inttypes.h>";
    print "\n#include <stdio.h>";
    print "\n#include <bits/wordsize.h>";
    print "\n#if __WORDSIZE == 64";
    print "\ntypedef uint64_t c_t;";
    print "\n#define U(n) UINT64_C (n)";
    print "\n#define PRI PRId64";
    print "\n#else";
    print "\ntypedef uint32_t c_t;";
    print "\n#define U(n) UINT32_C (n)";
    print "\n#define PRI PRId32";
    print "\n#endif";
    print "\nstatic int do_test (void)\n{\n  int bad = 0, good = 0;\n";
    print "#define TEST(name, source, expr) \\\n" \
      "  if (U (asconst_##name) != (c_t) (expr)) { ++bad;" \
      " fprintf (stderr, \"%s: %s is %\" PRI \" but %s is %\"PRI \"\\n\"," \
      " source, #name, U (asconst_##name), #expr, (c_t) (expr));" \
      " } else ++good;\n";
  }
  else
    print "void dummy(void) {";
  started = 1;
}

# Separator.
$1 == "--" { next }

NF == 1 { sub(/^.*$/, "& &"); }

NF > 1 {
  name = $1;
  sub(/^[^ 	]+[ 	]+/, "");
  if (test)
    print "  TEST (" name ", \"" FILENAME ":" FNR "\", " $0 ")";
  else
    printf "asm (\"@@@name@@@%s@@@value@@@%%0@@@end@@@\" : : \"i\" ((long) %s));\n",
      name, $0;
}

END {
  if (test) {
    print "  printf (\"%d errors in %d tests\\n\", bad, good + bad);"
    print "  return bad != 0 || good == 0;\n}\n";
    print "#define TEST_FUNCTION do_test ()";
  }
  else if (started) print "}";
}
