#! /usr/bin/perl

use Getopt::Long;
use POSIX;

$CC = "gcc";

$dialect="XOPEN2K";
GetOptions ('headers=s' => \@headers, 'dialect=s' => \$dialect);
@headers = split(/,/,join(',',@headers));

# List of the headers we are testing.
if (@headers == ()) {
  @headers = ("wordexp.h", "wctype.h", "wchar.h", "varargs.h", "utmpx.h",
	      "utime.h", "unistd.h", "ulimit.h", "ucontext.h", "time.h",
	      "tgmath.h", "termios.h", "tar.h", "sys/wait.h", "sys/utsname.h",
	      "sys/un.h", "sys/uio.h", "sys/types.h", "sys/times.h",
	      "sys/timeb.h", "sys/time.h", "sys/statvfs.h", "sys/stat.h",
	      "sys/socket.h", "sys/shm.h", "sys/sem.h", "sys/select.h",
	      "sys/resource.h", "sys/msg.h", "sys/mman.h", "sys/ipc.h",
	      "syslog.h", "stropts.h", "strings.h", "string.h", "stdlib.h",
	      "stdio.h", "stdint.h", "stddef.h", "stdarg.h", "spawn.h",
	      "signal.h", "setjmp.h", "semaphore.h", "search.h", "sched.h",
	      "regex.h", "pwd.h", "pthread.h", "poll.h", "nl_types.h",
	      "netinet/tcp.h", "netinet/in.h", "net/if.h", "netdb.h", "ndbm.h",
	      "mqueue.h", "monetary.h", "math.h", "locale.h", "libgen.h",
	      "limits.h", "langinfo.h", "iso646.h", "inttypes.h", "iconv.h",
	      "grp.h", "glob.h", "ftw.h", "fnmatch.h", "fmtmsg.h", "float.h",
	      "fcntl.h", "errno.h", "dlfcn.h", "dirent.h", "ctype.h", "cpio.h",
	      "complex.h", "assert.h", "arpa/inet.h", "aio.h");
}

if ($dialect ne "ISO" && $dialect ne "POSIX" && $dialect ne "XPG3"
    && $dialect ne "XPG4" && $dialect ne "UNIX98" && $dialect ne "XOPEN2K"
    && $dialect ne "XOPEN2K8" && $dialect ne "POSIX2008") {
  die "unknown dialect \"$dialect\"";
}

$CFLAGS{"ISO"} = "-I. -fno-builtin '-D__attribute__(x)=' -ansi";
$CFLAGS{"POSIX"} = "-I. -fno-builtin '-D__attribute__(x)=' -D_POSIX_C_SOURCE=199912";
$CFLAGS{"XPG3"} = "-I. -fno-builtin '-D__attribute__(x)=' -D_XOPEN_SOURCE";
$CFLAGS{"XPG4"} = "-I. -fno-builtin '-D__attribute__(x)=' -D_XOPEN_SOURCE_EXTENDED";
$CFLAGS{"UNIX98"} = "-I. -fno-builtin '-D__attribute__(x)=' -D_XOPEN_SOURCE=500";
$CFLAGS{"XOPEN2K"} = "-I. -fno-builtin '-D__attribute__(x)=' -D_XOPEN_SOURCE=600";
$CFLAGS{"XOPEN2K8"} = "-I. -fno-builtin '-D__attribute__(x)=' -D_XOPEN_SOURCE=700";
$CFLAGS{"POSIX2008"} = "-I. -fno-builtin '-D__attribute__(x)=' -D_POSIX_C_SOURCE=200809L";


# These are the ISO C99 keywords.
@keywords = ('auto', 'break', 'case', 'char', 'const', 'continue', 'default',
	     'do', 'double', 'else', 'enum', 'extern', 'float', 'for', 'goto',
	     'if', 'inline', 'int', 'long', 'register', 'restrict', 'return',
	     'short', 'signed', 'sizeof', 'static', 'struct', 'switch',
	     'typedef', 'union', 'unsigned', 'void', 'volatile', 'while');

# These are symbols which are known to pollute the namespace.
@knownproblems = ('unix', 'linux', 'i386');

if ($dialect ne "XOPEN2K8" && $dialect ne "POSIX2008") {
  # Some headers need a bit more attention.  At least with XPG7
  # all headers should be self-contained.
  $mustprepend{'inttypes.h'} = "#include <stddef.h>\n";
  $mustprepend{'regex.h'} = "#include <sys/types.h>\n";
  $mustprepend{'sched.h'} = "#include <sys/types.h>\n";
  $mustprepend{'signal.h'} = "#include <pthread.h>\n";
  $mustprepend{'stdio.h'} = "#include <sys/types.h>\n";
  $mustprepend{'wchar.h'} = "#include <stdarg.h>\n";
  $mustprepend{'wordexp.h'} = "#include <stddef.h>\n";
}

# Make a hash table from this information.
while ($#keywords >= 0) {
  $iskeyword{pop (@keywords)} = 1;
}

# Make a hash table from the known problems.
while ($#knownproblems >= 0) {
  $isknown{pop (@knownproblems)} = 1;
}

$uid = getuid();
($pwname,$pwpasswd,$pwuid,$pwgid,
 $pwquota,$pwcomment,$pwgcos,$pwdir,$pwshell,$pwexpire) = getpwuid($uid);
$tmpdir = "$pwdir";

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
    my($patlen) = length ($pattern) - 1;
    $res = ($strlen >= $patlen
	    && substr ($pattern, 0, $patlen) eq substr ($string, 0, $patlen));
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
    $ret = system "$CC $CFLAGS{$dialect} -c $fnamebase.c -o $fnamebase.o > $fnamebase.out 2>&1";
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
    $ret = system "$CC $CFLAGS{$dialect} -o $fnamebase $fnamebase.c > $fnamebase.out 2>&1";
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

  if ($isknown{$token}) {
    ++$nknown;
  } else {
    $errors{$token} = 1;
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
  open (CONTENT, "$CC $CFLAGS{$dialect} -E $fnamebase.c -P -Wp,-dN | sed -e '/^# [1-9]/d' -e '/^[[:space:]]*\$/d' |");
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
    if ($nknown > 0) {
      printf ("EXPECTED FAILURES\n");
      ++$known;
    } else {
      printf ("OK\n");
    }
  }
}


while ($#headers >= 0) {
  my($h) = pop (@headers);
  my($hf) = $h;
  $hf =~ s|/|-|;
  my($fnamebase) = "$tmpdir/$hf-test";
  my($missing);
  my(@allow) = ();
  my(@allowheader) = ();
  my(%seenheader) = ();
  my($prepend) = $mustprepend{$h};

  printf ("Testing <$h>\n");
  printf ("----------" . "-" x length ($h) . "\n");

  # Generate a program to test for the availability of this header.
  open (TESTFILE, ">$fnamebase.c");
  print TESTFILE "$prepend";
  print TESTFILE "#include <$h>\n";
  close (TESTFILE);

  $missing = compiletest ($fnamebase, "Checking whether <$h> is available",
			  "Header <$h> not available", 0, 0);

  printf ("\n");

  open (CONTROL, "$CC -E -D$dialect - < data/$h-data |");
  control: while (<CONTROL>) {
    chop;
    next control if (/^#/);
    next control if (/^[	]*$/);

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
			  "Member \"$member\" not available.", $res, 0);


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
    } elsif (/^optional-element *({([^}]*)}|([^{ ]*)) *({([^}]*)}|([^{ ]*)) *([A-Za-z0-9_]*) *(.*)/) {
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
			  "NOT AVAILABLE.", $res, 1);

      if ($res == 0 || $missing != 0) {
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
    } elsif (/^optional-constant *([a-zA-Z0-9_]*) ([>=<]+) ([A-Za-z0-9_]*)/) {
      my($const) = $1;
      my($op) = $2;
      my($value) = $3;
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $const;

      # Generate a program to test for the availability of this constant.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "__typeof__ ($const) a = $const;\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Testing for constant $const",
			  "NOT PRESENT", $res, 1);

      if ($value ne "" && $res == 0) {
	# Generate a program to test for the value of this constant.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	# Negate the value since 0 means ok
	print TESTFILE "int main (void) { return !($const $op $value); }\n";
	close (TESTFILE);

	$res = runtest ($fnamebase, "Testing for value of constant $const",
			"Constant \"$const\" has not the right value.", $res);
      }
    } elsif (/^constant *([a-zA-Z0-9_]*) *([>=<]+) ([A-Za-z0-9_]*)/) {
      my($const) = $1;
      my($op) = $2;
      my($value) = $3;
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $const;

      # Generate a program to test for the availability of this constant.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "__typeof__ ($const) a = $const;\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Testing for constant $const",
			  "Constant \"$const\" not available.", $res, 0);

      if ($value ne "") {
	# Generate a program to test for the value of this constant.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	# Negate the value since 0 means ok
	print TESTFILE "int main (void) { return !($const $op $value); }\n";
	close (TESTFILE);

	$res = runtest ($fnamebase, "Testing for value of constant $const",
			"Constant \"$const\" has not the right value.", $res);
      }
    } elsif (/^typed-constant *([a-zA-Z0-9_]*) *({([^}]*)}|([^ ]*)) *([A-Za-z0-9_]*)?/) {
      my($const) = $1;
      my($type) = "$3$4";
      my($value) = $5;
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $const;

      # Generate a program to test for the availability of this constant.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "__typeof__ ($const) a = $const;\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Testing for constant $const",
			  "Constant \"$const\" not available.", $res, 0);

      # Test the types of the members.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "__typeof__ (($type) 0) a;\n";
      print TESTFILE "extern __typeof__ ($const) a;\n";
      close (TESTFILE);

      compiletest ($fnamebase, "Testing for type of constant $const",
		   "Constant \"$const\" does not have the correct type.",
		   $res, 0);

      if ($value ne "") {
	# Generate a program to test for the value of this constant.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	print TESTFILE "int main (void) { return $const != $value; }\n";
	close (TESTFILE);

	$res = runtest ($fnamebase, "Testing for value of constant $const",
			"Constant \"$const\" has not the right value.", $res);
      }
    } elsif (/^optional-constant *([a-zA-Z0-9_]*) *([A-Za-z0-9_]*)?/) {
      my($const) = $1;
      my($value) = $2;
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $const;

      # Generate a program to test for the availability of this constant.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "__typeof__ ($const) a = $const;\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Testing for constant $const",
			  "NOT PRESENT", $res, 1);

      if ($value ne "" && $res == 0) {
	# Generate a program to test for the value of this constant.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	print TESTFILE "int main (void) { return $const != $value; }\n";
	close (TESTFILE);

	$res = runtest ($fnamebase, "Testing for value of constant $const",
			"Constant \"$const\" has not the right value.", $res);
      }
    } elsif (/^constant *([a-zA-Z0-9_]*) *([A-Za-z0-9_]*)?/) {
      my($const) = $1;
      my($value) = $2;
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $const;

      # Generate a program to test for the availability of this constant.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "__typeof__ ($const) a = $const;\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Testing for constant $const",
			  "Constant \"$const\" not available.", $res, 0);

      if ($value ne "") {
	# Generate a program to test for the value of this constant.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	print TESTFILE "int main (void) { return $const != $value; }\n";
	close (TESTFILE);

	$res = runtest ($fnamebase, "Testing for value of constant $const",
			"Constant \"$const\" has not the right value.", $res);
      }
    } elsif (/^symbol *([a-zA-Z0-9_]*) *([A-Za-z0-9_]*)?/) {
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
    } elsif (/^typed-constant *([a-zA-Z0-9_]*) *({([^}]*)}|([^ ]*)) *([A-Za-z0-9_]*)?/) {
      my($const) = $1;
      my($type) = "$3$4";
      my($value) = $5;
      my($res) = $missing;

      # Remember that this name is allowed.
      push @allow, $const;

      # Generate a program to test for the availability of this constant.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "__typeof__ ($const) a = $const;\n";
      close (TESTFILE);

      $res = compiletest ($fnamebase, "Testing for constant $const",
			  "Constant \"$const\" not available.", $res, 0);

      # Test the types of the members.
      open (TESTFILE, ">$fnamebase.c");
      print TESTFILE "$prepend";
      print TESTFILE "#include <$h>\n";
      print TESTFILE "__typeof__ (($type) 0) a;\n";
      print TESTFILE "extern __typeof__ ($const) a;\n";
      close (TESTFILE);

      compiletest ($fnamebase, "Testing for type of constant $const",
		   "Constant \"$const\" does not have the correct type.",
		   $res, 0);

      if ($value ne "") {
	# Generate a program to test for the value of this constant.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	print TESTFILE "int main (void) { return $const != $value; }\n";
	close (TESTFILE);

	$res = runtest ($fnamebase, "Testing for value of constant $const",
			"Constant \"$const\" has not the right value.", $res);
      }
    } elsif (/^optional-type *({([^}]*)|([a-zA-Z0-9_]*))/) {
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

      # Generate a program to test for the availability of this constant.
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
		   "NOT AVAILABLE", $missing, 1);
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
		   "Type \"$type\" not available.", $missing, 0);
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
    } elsif (/^optional-function *({([^}]*)}|([a-zA-Z0-9_]*)) [(][*]([a-zA-Z0-9_]*) ([(].*[)])/) {
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
			  "NOT AVAILABLE", $res, 1);

      if ($res == 0 || $missing == 1) {
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
			  "Function \"$fname\" is not available.", $res, 0);

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
    } elsif (/^optional-function *({([^}]*)}|([a-zA-Z0-9_]*)) ([a-zA-Z0-9_]*) ([(].*[)])/) {
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
			  "NOT AVAILABLE", $res, 1);

      if ($res == 0 || $missing != 0) {
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
			  "Function \"$fname\" is not available.", $res, 0);

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

      $res = compiletest ($fnamebase, "Test availability of function $fname",
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

      compiletest ($fnamebase, "Test for type of function $fname",
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
    } elsif (/^optional-macro *([^	]*)/) {
      my($macro) = "$1";

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
		   "NOT PRESENT", $missing, 1);
    } elsif (/^macro *([a-zA-Z0-9_]*) *([>=<]+) ([A-Za-z0-9_]*)/) {
      my($macro) = "$1";
      my($op) = $2;
      my($value) = $3;
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

      $res = compiletest ($fnamebase, "Test availability of macro $macro",
			  "Macro \"$macro\" is not available.", $res, 0);

      if ($value ne "") {
	# Generate a program to test for the value of this constant.
	open (TESTFILE, ">$fnamebase.c");
	print TESTFILE "$prepend";
	print TESTFILE "#include <$h>\n";
	# Negate the value since 0 means ok
	print TESTFILE "int main (void) { return !($macro $op $value); }\n";
	close (TESTFILE);

	$res = runtest ($fnamebase, "Testing for value of constant $macro",
			"Macro \"$macro\" has not the right value.", $res);
      }
    } elsif (/^macro *([^	]*)/) {
      my($macro) = "$1";

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

    open (ALLOW, "$CC -E -D$dialect - < data/$ah-data |");
    acontrol: while (<ALLOW>) {
      chop;
      next acontrol if (/^#/);
      next acontrol if (/^[	]*$/);

      if (/^element *({([^}]*)}|([^ ]*)) *({([^}]*)}|([^ ]*)) *([A-Za-z0-9_]*) *(.*)/) {
	push @allow, $7;
      } elsif (/^constant *([a-zA-Z0-9_]*) *([A-Za-z0-9_]*)?/) {
	push @allow, $1;
      } elsif (/^typed-constant *([a-zA-Z0-9_]*) *({([^}]*)}|([^ ]*)) *([A-Za-z0-9_]*)?/) {
	push @allow, 1;
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
      } elsif (/^macro *([^	]*)/) {
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

  # Now check the namespace.
  printf ("  Checking the namespace of \"%s\"... ", $h);
  if ($missing) {
    ++$skipped;
    printf ("SKIP\n");
  } else {
    checknamespace ($h, $fnamebase, @allow);
  }

  printf ("\n\n");
}

printf "-" x 76 . "\n";
printf ("  Total number of tests   : %4d\n", $total);

printf ("  Number of known failures: %4d (", $known);
$percent = ($known * 100) / $total;
if ($known > 0 && $percent < 1.0) {
  printf (" <1%%)\n");
} else {
  printf ("%3d%%)\n", $percent);
}

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
