#! /usr/bin/perl

use Getopt::Long;
use POSIX;

$standard = "XOPEN2K8";
$CC = "gcc";
$tmpdir = "/tmp";
GetOptions ('headers=s' => \@headers, 'standard=s' => \$standard,
	    'flags=s' => \$flags, 'cc=s' => \$CC, 'tmpdir=s' => \$tmpdir);
@headers = split(/,/,join(',',@headers));

# List of the headers we are testing.
if (@headers == ()) {
  @headers = ("wordexp.h", "wctype.h", "wchar.h", "varargs.h", "utmpx.h",
	      "utime.h", "unistd.h", "ulimit.h", "ucontext.h", "uchar.h",
	      "time.h", "tgmath.h", "termios.h", "tar.h", "sys/wait.h",
	      "sys/utsname.h", "sys/un.h", "sys/uio.h", "sys/types.h",
	      "sys/times.h", "sys/timeb.h", "sys/time.h", "sys/statvfs.h",
	      "sys/stat.h", "sys/socket.h", "sys/shm.h", "sys/sem.h",
	      "sys/select.h", "sys/resource.h", "sys/msg.h", "sys/mman.h",
	      "sys/ipc.h", "syslog.h", "stropts.h", "strings.h", "string.h",
	      "stdnoreturn.h", "stdlib.h", "stdio.h", "stdint.h", "stddef.h",
	      "stdbool.h", "stdarg.h", "stdalign.h", "spawn.h", "signal.h",
	      "setjmp.h", "semaphore.h", "search.h", "sched.h", "regex.h",
	      "pwd.h", "pthread.h", "poll.h", "nl_types.h", "netinet/tcp.h",
	      "netinet/in.h", "net/if.h", "netdb.h", "ndbm.h", "mqueue.h",
	      "monetary.h", "math.h", "locale.h", "libgen.h", "limits.h",
	      "langinfo.h", "iso646.h", "inttypes.h", "iconv.h", "grp.h",
	      "glob.h", "ftw.h", "fnmatch.h", "fmtmsg.h", "float.h", "fenv.h",
	      "fcntl.h", "errno.h", "dlfcn.h", "dirent.h", "ctype.h", "cpio.h",
	      "complex.h", "assert.h", "arpa/inet.h", "aio.h");
}

$CFLAGS{"ISO"} = "-ansi";
$CFLAGS{"ISO99"} = "-std=c99";
$CFLAGS{"ISO11"} = "-std=c1x -D_ISOC11_SOURCE";
$CFLAGS{"POSIX"} = "-D_POSIX_C_SOURCE=199912 -ansi";
$CFLAGS{"XPG3"} = "-ansi -D_XOPEN_SOURCE";
$CFLAGS{"XPG4"} = "-ansi -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED";
$CFLAGS{"UNIX98"} = "-ansi -D_XOPEN_SOURCE=500";
$CFLAGS{"XOPEN2K"} = "-std=c99 -D_XOPEN_SOURCE=600";
$CFLAGS{"XOPEN2K8"} = "-std=c99 -D_XOPEN_SOURCE=700";
$CFLAGS{"POSIX2008"} = "-std=c99 -D_POSIX_C_SOURCE=200809L";

$CFLAGS_namespace = "$flags -fno-builtin $CFLAGS{$standard} -D_ISOMAC";
$CFLAGS = "$CFLAGS_namespace '-D__attribute__(x)='";

# Check standard name for validity.
die "unknown standard \"$standard\"" if ($CFLAGS{$standard} eq "");

# if ($standard ne "XOPEN2K8" && $standard ne "POSIX2008") {
#   # Some headers need a bit more attention.  At least with XPG7
#   # all headers should be self-contained.
#   $mustprepend{'inttypes.h'} = "#include <stddef.h>\n";
#   $mustprepend{'glob.h'} = "#include <sys/types.h>\n";
#   $mustprepend{'grp.h'} = "#include <sys/types.h>\n";
#   $mustprepend{'regex.h'} = "#include <sys/types.h>\n";
#   $mustprepend{'pwd.h'} = "#include <sys/types.h>\n";
#   $mustprepend{'sched.h'} = "#include <sys/types.h>\n";
#   $mustprepend{'signal.h'} = "#include <pthread.h>\n#include <sys/types.h>\n";
#   $mustprepend{'stdio.h'} = "#include <sys/types.h>\n";
#   $mustprepend{'sys/stat.h'} = "#include <sys/types.h>\n";
#   $mustprepend{'wchar.h'} = "#include <stdarg.h>\n";
#   $mustprepend{'wordexp.h'} = "#include <stddef.h>\n";
# }

# These are the ISO C90 keywords.
@keywords = ('auto', 'break', 'case', 'char', 'const', 'continue', 'default',
	     'do', 'double', 'else', 'enum', 'extern', 'float', 'for', 'goto',
	     'if', 'int', 'long', 'register', 'return',
	     'short', 'signed', 'sizeof', 'static', 'struct', 'switch',
	     'typedef', 'union', 'unsigned', 'void', 'volatile', 'while');
if ($CFLAGS{$standard} =~ /-std=(c99|c1x)/) {
  push (@keywords, 'inline', 'restrict');
}

# Make a hash table from this information.
while ($#keywords >= 0) {
  $iskeyword{pop (@keywords)} = 1;
}

$verbose = 1;

$total = 0;
$skipped = 0;
$errors = 0;


sub poorfnmatch {
  my($pattern, $string) = @_;
  my($strlen) = length ($string);
  my($res);

  if (substr ($pattern, 0, 1) eq '*') {
    my($patlen) = length ($pattern) - 1;
    $res = ($strlen >= $patlen
	    && substr ($pattern, -$patlen, $patlen) eq substr ($string, -$patlen, $patlen));
  } elsif (substr ($pattern, -1, 1) eq '*') {
    if (substr ($pattern, -2, 1) eq ']') {
      my($patlen) = index ($pattern, '[');
      my($range) = substr ($pattern, $patlen + 1, -2);
      $res = ($strlen > $patlen
	      && substr ($pattern, 0, $patlen) eq substr ($string, 0, $patlen)
	      && index ($range, substr ($string, $patlen, 1)) != -1);
    } else {
      my($patlen) = length ($pattern) - 1;
      $res = ($strlen >= $patlen
	      && substr ($pattern, 0, $patlen) eq substr ($string, 0, $patlen));
    }
  } else {
    $res = $pattern eq $string;
  }
  return $res;
}


sub compiletest
{
  my($fnamebase, $msg, $errmsg, $skip, $optional) = @_;
  my($result) = $skip;
  my($printlog) = 0;

  ++$total;
  printf ("  $msg...");

  if ($skip != 0) {
    ++$skipped;
    printf (" SKIP\n");
  } else {
    $ret = system "$CC $CFLAGS -c $fnamebase.c -o $fnamebase.o > $fnamebase.out 2>&1";
    if ($ret != 0) {
      if ($optional != 0) {
	printf (" $errmsg\n");
	$result = 1;
      } else {
	printf (" FAIL\n");
	if ($verbose != 0) {
	  printf ("    $errmsg  Compiler message:\n");
	  $printlog = 1;
	}
	++$errors;
	$result = 1;
      }
    } else {
      printf (" OK\n");
      if ($verbose > 1 && -s "$fnamebase.out") {
	# We print all warnings issued.
	$printlog = 1;
      }
    }
    if ($printlog != 0) {
      printf ("    " . "-" x 71 . "\n");
      open (MESSAGE, "< $fnamebase.out");
      while (<MESSAGE>) {
	printf ("    %s", $_);
      }
      close (MESSAGE);
      printf ("    " . "-" x 71 . "\n");
    }
  }
  unlink "$fnamebase.c";
  unlink "$fnamebase.o";
  unlink "$fnamebase.out";

  $result;
}


sub runtest
{
  my($fnamebase, $msg, $errmsg, $skip) = @_;
  my($result) = $skip;
  my($printlog) = 0;

  ++$total;
  printf ("  $msg...");

  if ($skip != 0) {
    ++$skipped;
    printf (" SKIP\n");
  } else {
    $ret = system "$CC $CFLAGS -o $fnamebase $fnamebase.c > $fnamebase.out 2>&1";
    if ($ret != 0) {
      printf (" FAIL\n");
      if ($verbose != 0) {
	printf ("    $errmsg  Compiler message:\n");
	$printlog = 1;
      }
      ++$errors;
      $result = 1;
    } else {
      # Now run the program.  If the exit code is not zero something is wrong.
      $result = system "$fnamebase > $fnamebase.out2 2>&1";
      if ($result == 0) {
	printf (" OK\n");
	if ($verbose > 1 && -s "$fnamebase.out") {
	  # We print all warnings issued.
	  $printlog = 1;
	  system "cat $fnamebase.out2 >> $fnamebase.out";
	}
      } else {
	printf (" FAIL\n");
	++$errors;
	$printlog = 1;
	unlink "$fnamebase.out";
	rename "$fnamebase.out2", "$fnamebase.out";
      }
    }
    if ($printlog != 0) {
      printf ("    " . "-" x 71 . "\n");
      open (MESSAGE, "< $fnamebase.out");
      while (<MESSAGE>) {
	printf ("    %s", $_);
      }
      close (MESSAGE);
      printf ("    " . "-" x 71 . "\n");
    }
  }
  unlink "$fnamebase";
  unlink "$fnamebase.c";
  unlink "$fnamebase.o";
  unlink "$fnamebase.out";
  unlink "$fnamebase.out2";

  $result;
}


sub newtoken {
  my($token, @allow) = @_;
  my($idx);

  return if ($token =~ /^[0-9_]/ || $iskeyword{$token});

  for ($idx = 0; $idx <= $#allow; ++$idx) {
    return if (poorfnmatch ($allow[$idx], $token));
  }
}


sub removetoken {
  my($token) = @_;
  my($idx);

  return if ($token =~ /^[0-9_]/ || $iskeyword{$token});

  if (exists $errors{$token}) {
    undef $errors{$token};
  }
}


sub checknamespace {
  my($h, $fnamebase, @allow) = @_;

  ++$total;

  # Generate a program to get the contents of this header.
  open (TESTFILE, ">$fnamebase.c");
  print TESTFILE "#include <$h>\n";
  close (TESTFILE);

  undef %errors;
  $nknown = 0;
  open (CONTENT, "$CC $CFLAGS_namespace -E $fnamebase.c -P -Wp,-dN | sed -e '/^# [1-9]/d' -e '/^[[:space:]]*\$/d' |");
  loop: while (<CONTENT>) {
    chop;
    if (/^#define (.*)/) {
      newtoken ($1, @allow);
    } elsif (/^#undef (.*)/) {
      removetoken ($1);
    } else {
      # We have to tokenize the line.
      my($str) = $_;
      my($index) = 0;
      my($len) = length ($str);

      foreach $token (split(/[^a-zA-Z0-9_]/, $str)) {
	if ($token ne "") {
	  newtoken ($token, @allow);
	}
      }
    }
  }
  close (CONTENT);
  unlink "$fnamebase.c";
  $realerror = 0;
  if ($#errors != 0) {
    # Sort the output list so it's easier to compare results with diff.
    foreach $f (sort keys(%errors)) {
      if ($errors{$f} == 1) {
	if ($realerror == 0) {
	  printf ("FAIL\n    " . "-" x 72 . "\n");
	  $realerror = 1;
	  ++$errors;
	}
	printf ("    Namespace violation: \"%s\"\n", $f);
      }
    }
    printf ("    " . "-" x 72 . "\n") if ($realerror != 0);
  }

  if ($realerror == 0) {
    printf ("OK\n");
  }
}


while ($#headers >= 0) {
  my($h) = pop (@headers);
  my($hf) = $h;
  $hf =~ s|/|-|;
  my($fnamebase) = "$tmpdir/$hf-test";
  my($missing) = 1;
  my(@allow) = ();
  my(@allowheader) = ();
  my(%seenheader) = ();
  my($prepend) = $mustprepend{$h};
  my($test_exist) = 1;

  printf ("Testing <$h>\n");
  printf ("----------" . "-" x length ($h) . "\n");

  open (CONTROL, "$CC -E -D$standard -x c data/$h-data |");
  control: while (<CONTROL>) {
    chop;
    next control if (/^#/);
    next control if (/^[	]*$/);

    if ($test_exist) {
      $test_exist = 0;
      # Generate a program to test for the availability of this header.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      close (TESTFILE);

      $missing = compiletest ($fnamebase, "Checking whether <$h> is available",
			      "Header <$h> not available", 0, 0);
      printf ("\n");
      last control if ($missing);
    }

    my($optional) = 0;
    if (/^optional-/) {
      s/^optional-//;
      $optional = 1;
    }
    if (/^element *({([^}]*)}|([^{ ]*)) *({([^}]*)}|([^{ ]*)) *([A-Za-z0-9_]*) *(.*)/) {
      my($struct) = "$2$3";
      my($type) = "$5$6";
      my($member) = "$7";
      my($rest) = "$8";
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $member;

      # Generate a program to test for the availability of this member.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "$struct a;\n";
      print TESTFILE "$struct b;\n";
      print TESTFILE "extern void xyzzy (__typeof__ (&b.$member), __typeof__ (&a.$member), unsigned);\n";
      print TESTFILE "void foobarbaz (void) {\n";
      print TESTFILE "  xyzzy (&a.$member, &b.$member, sizeof (a.$member));\n";
      print TESTFILE "}\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Testing for member $member",
			  ($optional
			   ? "NOT AVAILABLE."
			   : "Member \"$member\" not available."), $res,
			  $optional);

      if ($res == 0 || $missing != 0 || !$optional) {
	# Test the types of the members.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	print TESTFILE "$struct a;\n";
	print TESTFILE "extern $type b$rest;\n";
	print TESTFILE "extern __typeof__ (a.$member) b;\n";
	close (TESTFILE);

	compiletest ($fnamebase, "Testing for type of member $member",
		     "Member \"$member\" does not have the correct type.",
		     $res, 0);
      }
    } elsif (/^(macro|constant|macro-constant|macro-int-constant) +([a-zA-Z0-9_]*) *(?:{([^}]*)} *)?(?:([>=<!]+) ([A-Za-z0-9_-]*))?/) {
      my($symbol_type) = $1;
      my($symbol) = $2;
      my($type) = $3;
      my($op) = $4;
      my($value) = $5;
      my($res) = $missing;
      my($mres) = $missing;
      my($cres) = $missing;

      # Remember that this name is allowed.
      push @allow, $symbol;

      if ($symbol_type =~ /macro/) {
	# Generate a program to test for availability of this macro.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	print TESTFILE "#ifndef $symbol\n";
	print TESTFILE "# error \"Macro $symbol not defined\"\n";
	print TESTFILE "#endif\n";
	close (TESTFILE);

	$mres = compiletest ($fnamebase, "Test availability of macro $symbol",
			     ($optional
			      ? "NOT PRESENT"
			      : "Macro \"$symbol\" is not available."), $res,
			     $optional);
      }

      if ($symbol_type =~ /constant/) {
	# Generate a program to test for the availability of this constant.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	print TESTFILE "__typeof__ ($symbol) a = $symbol;\n";
	close (TESTFILE);

	$cres = compiletest ($fnamebase, "Testing for constant $symbol",
			     ($optional
			      ? "NOT PRESENT"
			      : "Constant \"$symbol\" not available."), $res,
			     $optional);
      }

      $res = $res || $mres || $cres;

      if ($symbol_type eq "macro-int-constant" && ($res == 0 || !$optional)) {
	# Test that the symbol is usable in #if.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	print TESTFILE "#if $symbol < 0\n";
	print TESTFILE "# define conformtest_negative 1\n";
	my($s) = "0";
	for (my $i = 0; $i < 63; $i++) {
	  print TESTFILE "# if $symbol & (1LL << $i)\n";
	  print TESTFILE "#  define conformtest_bit_$i 0LL\n";
	  print TESTFILE "# else\n";
	  print TESTFILE "#  define conformtest_bit_$i (1LL << $i)\n";
	  print TESTFILE "# endif\n";
	  $s .= "|conformtest_bit_$i";
	}
	print TESTFILE "# define conformtest_value ~($s)\n";
	print TESTFILE "#else\n";
	print TESTFILE "# define conformtest_negative 0\n";
	$s = "0";
	for (my $i = 0; $i < 64; $i++) {
	  print TESTFILE "# if $symbol & (1ULL << $i)\n";
	  print TESTFILE "#  define conformtest_bit_$i (1ULL << $i)\n";
	  print TESTFILE "# else\n";
	  print TESTFILE "#  define conformtest_bit_$i 0ULL\n";
	  print TESTFILE "# endif\n";
	  $s .= "|conformtest_bit_$i";
	}
	print TESTFILE "# define conformtest_value ($s)\n";
	print TESTFILE "#endif\n";
	print TESTFILE "int main (void) { return !((($symbol < 0) == conformtest_negative) && ($symbol == conformtest_value)); }\n";
	close (TESTFILE);

	runtest ($fnamebase, "Testing for #if usability of symbol $symbol",
		 "Symbol \"$symbol\" not usable in #if.", $res);
      }

      if (defined ($type) && ($res == 0 || !$optional)) {
	# Test the type of the symbol.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	if ($type =~ /^promoted:/) {
	  $type =~ s/^promoted://;
	  print TESTFILE "__typeof__ (($type) 0 + ($type) 0) a;\n";
	} else {
	  print TESTFILE "__typeof__ (($type) 0) a;\n";
	}
	print TESTFILE "extern __typeof__ ($symbol) a;\n";
	close (TESTFILE);

	compiletest ($fnamebase, "Testing for type of symbol $symbol",
		     "Symbol \"$symbol\" does not have the correct type.",
		     $res, 0);
      }

      if (defined ($op) && ($res == 0 || !$optional)) {
	# Generate a program to test for the value of this symbol.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	# Negate the value since 0 means ok
	print TESTFILE "int main (void) { return !($symbol $op $value); }\n";
	close (TESTFILE);

	$res = runtest ($fnamebase, "Testing for value of symbol $symbol",
			"Symbol \"$symbol\" has not the right value.", $res);
      }
    } elsif (/^symbol *([a-zA-Z0-9_]*) *([A-Za-z0-9_-]*)?/) {
      my($symbol) = $1;
      my($value) = $2;
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $symbol;

      # Generate a program to test for the availability of this constant.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "void foobarbaz (void) {\n";
      print TESTFILE "__typeof__ ($symbol) a = $symbol;\n";
      print TESTFILE "}\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Testing for symbol $symbol",
			  "Symbol \"$symbol\" not available.", $res, 0);

      if ($value ne "") {
	# Generate a program to test for the value of this constant.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	print TESTFILE "int main (void) { return $symbol != $value; }\n";
	close (TESTFILE);

	$res = runtest ($fnamebase, "Testing for value of symbol $symbol",
			"Symbol \"$symbol\" has not the right value.", $res);
      }
    } elsif (/^type *({([^}]*)|([a-zA-Z0-9_]*))/) {
      my($type) = "$2$3";
      my($maybe_opaque) = 0;

      # Remember that this name is allowed.
      if ($type =~ /^struct *(.*)/) {
	push @allow, $1;
      } elsif ($type =~ /^union *(.*)/) {
	push @allow, $1;
      } else {
	push @allow, $type;
	$maybe_opaque = 1;
      }

      # Generate a program to test for the availability of this type.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      if ($maybe_opaque == 1) {
	print TESTFILE "$type *a;\n";
      } else {
	print TESTFILE "$type a;\n";
      }
      close (TESTFILE);

      compiletest ($fnamebase, "Testing for type $type",
		   ($optional
		    ? "NOT AVAILABLE"
		    : "Type \"$type\" not available."), $missing, $optional);
    } elsif (/^tag *({([^}]*)|([a-zA-Z0-9_]*))/) {
      my($type) = "$2$3";

      # Remember that this name is allowed.
      if ($type =~ /^struct *(.*)/) {
	push @allow, $1;
      } elsif ($type =~ /^union *(.*)/) {
	push @allow, $1;
      } else {
	push @allow, $type;
      }

      # Generate a program to test for the availability of this type.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "$type;\n";
      close (TESTFILE);

      compiletest ($fnamebase, "Testing for type $type",
		   "Type \"$type\" not available.", $missing, 0);
    } elsif (/^function *({([^}]*)}|([a-zA-Z0-9_]*)) [(][*]([a-zA-Z0-9_]*) ([(].*[)])/) {
      my($rettype) = "$2$3";
      my($fname) = "$4";
      my($args) = "$5";
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $fname;

      # Generate a program to test for availability of this function.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      # print TESTFILE "#undef $fname\n";
      print TESTFILE "$rettype (*(*foobarbaz) $args = $fname;\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Test availability of function $fname",
			  ($optional
			   ? "NOT AVAILABLE"
			   : "Function \"$fname\" is not available."), $res,
			  $optional);

      if ($res == 0 || $missing == 1 || !$optional) {
	# Generate a program to test for the type of this function.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	# print TESTFILE "#undef $fname\n";
	print TESTFILE "extern $rettype (*(*foobarbaz) $args;\n";
	print TESTFILE "extern __typeof__ (&$fname) foobarbaz;\n";
	close (TESTFILE);

	compiletest ($fnamebase, "Test for type of function $fname",
		     "Function \"$fname\" has incorrect type.", $res, 0);
      }
    } elsif (/^function *({([^}]*)}|([a-zA-Z0-9_]*)) ([a-zA-Z0-9_]*) ([(].*[)])/) {
      my($rettype) = "$2$3";
      my($fname) = "$4";
      my($args) = "$5";
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $fname;

      # Generate a program to test for availability of this function.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      # print TESTFILE "#undef $fname\n";
      print TESTFILE "$rettype (*foobarbaz) $args = $fname;\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Test availability of function $fname",
			  ($optional
			   ? "NOT AVAILABLE"
			   : "Function \"$fname\" is not available."), $res,
			  $optional);

      if ($res == 0 || $missing != 0 || !$optional) {
	# Generate a program to test for the type of this function.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	# print TESTFILE "#undef $fname\n";
	print TESTFILE "extern $rettype (*foobarbaz) $args;\n";
	print TESTFILE "extern __typeof__ (&$fname) foobarbaz;\n";
	close (TESTFILE);

	compiletest ($fnamebase, "Test for type of function $fname",
		     "Function \"$fname\" has incorrect type.", $res, 0);
      }
    } elsif (/^variable *({([^}]*)}|([a-zA-Z0-9_]*)) ([a-zA-Z0-9_]*) *(.*)/) {
      my($type) = "$2$3";
      my($vname) = "$4";
      my($rest) = "$5";
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $vname;

      # Generate a program to test for availability of this function.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      # print TESTFILE "#undef $fname\n";
      print TESTFILE "typedef $type xyzzy$rest;\n";
      print TESTFILE "$xyzzy *foobarbaz = &$vname;\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Test availability of variable $vname",
			  "Variable \"$vname\" is not available.", $res, 0);

      # Generate a program to test for the type of this function.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      # print TESTFILE "#undef $fname\n";
      print TESTFILE "extern $type $vname$rest;\n";
      close (TESTFILE);

      compiletest ($fnamebase, "Test for type of variable $fname",
		   "Variable \"$vname\" has incorrect type.", $res, 0);
    } elsif (/^macro-function *({([^}]*)}|([a-zA-Z0-9_]*)) ([a-zA-Z0-9_]*) ([(].*[)])/) {
      my($rettype) = "$2$3";
      my($fname) = "$4";
      my($args) = "$5";
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $fname;

      # Generate a program to test for availability of this function.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "#ifndef $fname\n";
      print TESTFILE "$rettype (*foobarbaz) $args = $fname;\n";
      print TESTFILE "#endif\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Test availability of macro $fname",
			  "Function \"$fname\" is not available.", $res, 0);

      # Generate a program to test for the type of this function.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "#ifndef $fname\n";
      print TESTFILE "extern $rettype (*foobarbaz) $args;\n";
      print TESTFILE "extern __typeof__ (&$fname) foobarbaz;\n";
      print TESTFILE "#endif\n";
      close (TESTFILE);

      compiletest ($fnamebase, "Test for type of macro $fname",
		   "Function \"$fname\" has incorrect type.", $res, 0);
    } elsif (/^macro-str *([^	 ]*) *(\".*\")/) {
      # The above regex doesn't handle a \" in a string.
      my($macro) = "$1";
      my($string) = "$2";
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $macro;

      # Generate a program to test for availability of this macro.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "#ifndef $macro\n";
      print TESTFILE "# error \"Macro $macro not defined\"\n";
      print TESTFILE "#endif\n";
      close (TESTFILE);

      compiletest ($fnamebase, "Test availability of macro $macro",
		   "Macro \"$macro\" is not available.", $missing, 0);

      # Generate a program to test for the value of this macro.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      # We can't include <string.h> here.
      print TESTFILE "extern int (strcmp)(const char *, const char *);\n";
      print TESTFILE "int main (void) { return (strcmp) ($macro, $string) != 0;}\n";
      close (TESTFILE);

      $res = runtest ($fnamebase, "Testing for value of macro $macro",
		      "Macro \"$macro\" has not the right value.", $res);
    } elsif (/^allow-header *(.*)/) {
      my($pattern) = $1;
      if ($seenheader{$pattern} != 1) {
	push @allowheader, $pattern;
	$seenheader{$pattern} = 1;
      }
      next control;
    } elsif (/^allow *(.*)/) {
      my($pattern) = $1;
      push @allow, $pattern;
      next control;
    } else {
      # printf ("line is `%s'\n", $_);
      next control;
    }

    printf ("\n");
  }
  close (CONTROL);

  # Read the data files for the header files which are allowed to be included.
  while ($#allowheader >= 0) {
    my($ah) = pop @allowheader;

    open (ALLOW, "$CC -E -D$standard -x c data/$ah-data |");
    acontrol: while (<ALLOW>) {
      chop;
      next acontrol if (/^#/);
      next acontrol if (/^[	]*$/);

      if (/^element *({([^}]*)}|([^ ]*)) *({([^}]*)}|([^ ]*)) *([A-Za-z0-9_]*) *(.*)/) {
	push @allow, $7;
      } elsif (/^(macro|constant|macro-constant|macro-int-constant) +([a-zA-Z0-9_]*) *(?:{([^}]*)} *)?(?:([>=<!]+) ([A-Za-z0-9_-]*))?/) {
	push @allow, $1;
      } elsif (/^(type|tag) *({([^}]*)|([a-zA-Z0-9_]*))/) {
	my($type) = "$3$4";

	# Remember that this name is allowed.
	if ($type =~ /^struct *(.*)/) {
	  push @allow, $1;
	} elsif ($type =~ /^union *(.*)/) {
	  push @allow, $1;
	} else {
	  push @allow, $type;
	}
      } elsif (/^function *({([^}]*)}|([a-zA-Z0-9_]*)) [(][*]([a-zA-Z0-9_]*) ([(].*[)])/) {
	push @allow, $4;
      } elsif (/^function *({([^}]*)}|([a-zA-Z0-9_]*)) ([a-zA-Z0-9_]*) ([(].*[)])/) {
	push @allow, $4;
      } elsif (/^variable *({([^}]*)}|([a-zA-Z0-9_]*)) ([a-zA-Z0-9_]*)/) {
	push @allow, $4;
      } elsif (/^macro-function *({([^}]*)}|([a-zA-Z0-9_]*)) ([a-zA-Z0-9_]*) ([(].*[)])/) {
	push @allow, $4;
      } elsif (/^symbol *([a-zA-Z0-9_]*) *([A-Za-z0-9_-]*)?/) {
	push @allow, $1;
      } elsif (/^allow-header *(.*)/) {
	if ($seenheader{$1} != 1) {
	  push @allowheader, $1;
	  $seenheader{$1} = 1;
	}
      } elsif (/^allow *(.*)/) {
	push @allow, $1;
      }
    }
    close (ALLOW);
  }

  if ($test_exist) {
    printf ("  Not defined\n");
  } else {
    # Now check the namespace.
    printf ("  Checking the namespace of \"%s\"... ", $h);
    if ($missing) {
      ++$skipped;
      printf ("SKIP\n");
    } else {
      checknamespace ($h, $fnamebase, @allow);
    }
  }

  printf ("\n\n");
}

printf "-" x 76 . "\n";
printf ("  Total number of tests   : %4d\n", $total);

printf ("  Number of failed tests  : %4d (", $errors);
$percent = ($errors * 100) / $total;
if ($errors > 0 && $percent < 1.0) {
  printf (" <1%%)\n");
} else {
  printf ("%3d%%)\n", $percent);
}

printf ("  Number of skipped tests : %4d (", $skipped);
$percent = ($skipped * 100) / $total;
if ($skipped > 0 && $percent < 1.0) {
  printf (" <1%%)\n");
} else {
  printf ("%3d%%)\n", $percent);
}

exit $errors != 0;
# Local Variables:
#  perl-indent-level: 2
# End:
