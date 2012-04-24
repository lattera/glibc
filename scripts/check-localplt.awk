# This is an awk script to process the output of elf/check-localplt.
# The first file argument is the file of expected results.
# Each line is either a comment starting with # or it looks like:
#	libfoo.so: function
# or
#	libfoo.so: function ?
# The latter means that a PLT entry for function is optional in libfoo.so.
# The former means one is required.
# The second file argument is - and this (stdin) receives the output
# of the check-localplt program.

BEGIN { result = 0 }

FILENAME != "-" && /^#/ { next }

FILENAME != "-" {
  if (NF != 2 && !(NF == 3 && $3 == "?")) {
    printf "%s:%d: bad data line: %s\n", FILENAME, FNR, $0 > "/dev/stderr";
    result = 2;
  } else {
    accept[$1 " " $2] = NF == 2;
  }
  next;
}

NF != 2 {
  print "Unexpected output from check-localplt:", $0 > "/dev/stderr";
  result = 2;
  next
}

{
  key = $1 " " $2
  if (key in accept) {
    delete accept[key]
  } else {
    print "Extra PLT reference:", $0;
    if (result == 0)
      result = 1;
  }
}

END {
  for (key in accept) {
    if (accept[key]) {
      # It's mandatory.
      print "Missing required PLT reference:", key;
      result = 1;
    }
  }

  exit(result);
}
