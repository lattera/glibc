# This awk script processes the output of objdump --dynamic-syms
# into a simple format that should not change when the ABI is not changing.

BEGIN {
  if (parse_names)
    defout = "/dev/stderr";
  else
    defout = "/dev/stdout";
}

# Per-file header.
/[^ :]+\.so\.[0-9]+:[ 	]+.file format .*$/ {
  if (parse_names && soname != "")
    emit(1);

  sofullname = $1;
  sub(/:$/, "", sofullname);
  soname = sofullname;
  sub(/^.*\//, "", soname);
  sub(/\.so\.[0-9]+$/, "", soname);

  next
}

# Normalize columns.
/^[0-9a-fA-F]+      / { sub(/      /, "  -   ") }

# Skip undefineds.
$4 == "*UND*" { next }

# Skip locals.
$2 == "l" { next }

$2 == "g" || $2 == "w" && NF == 7 {
  weak = $2;
  type = $3;
  size = $5;
  sub(/^0*/, "", size);
  size = " 0x" size;
  version = $6;
  symbol = $7;
  gsub(/[()]/, "", version);

  if (version == "GLIBC_PRIVATE") next;

  if (type == "D" && $4 == ".tbss") {
    type = "T";
  }
  else if (type == "D" && $4 == ".opd") {
    type = "O";
    size = "";
  }
  else if (type == "DO" && $4 == "*ABS*") {
    type = "A";
    size = "";
  }
  else if (type == "DO") {
    type = "D";
  }
  else if (type == "DF") {
    type = "F";
    size = "";
  }
  else {
    print symbol, version, weak, "?", type, $4, $5 > defout;
    next;
  }
  if (size == " 0x") {
    print symbol, version, weak, "?", type, $4, $5 > defout;
    next;
  }

  # Disabled -- weakness should not matter to shared library ABIs any more.
  #if (weak == "w") type = tolower(type);
  desc = " " symbol " " type size;

  if (version in versions) {
    versions[version] = versions[version] "\n" desc;
  }
  else {
    versions[version] = desc;
  }
  next;
}

# Header crapola.
NF == 0 || /DYNAMIC SYMBOL TABLE/ || /file format/ { next }

{
  print "Don't grok this line:", $0 > defout
}

function emit(tofile) {
  nverlist = 0;
  for (version in versions) {
    if (nverslist == 0) {
      verslist = version;
      nverslist = 1;
      continue;
    }
    split(verslist, s, "\n");
    if (version < s[1]) {
      verslist = version;
      for (i = 1; i <= nverslist; ++i) {
	verslist = verslist "\n" s[i];
      }
    }
    else {
      verslist = s[1];
      for (i = 2; i <= nverslist; ++i) {
	if (version < s[i]) break;
	verslist = verslist "\n" s[i];
      }
      verslist = verslist "\n" version;
      for (; i <= nverslist; ++i) {
	verslist = verslist "\n" s[i];
      }
    }
    ++nverslist;
  }

  if (tofile) {
    out = prefix soname ".symlist";
    if (soname in outfiles)
      out = out "." ++outfiles[soname];
    else
      outfiles[soname] = 1;
    printf "" > out;
  }

  split(verslist, order, "\n");
  for (i = 1; i <= nverslist; ++i) {
    version = order[i];

    if (tofile) {
      print version >> out;
      close(out);
      outpipe = "sort >> " out;
    }
    else
      outpipe = "sort";
    print versions[version] | outpipe;
    close(outpipe);

    delete versions[version];
  }
  for (version in versions)
    delete versions[version];

  if (tofile)
    print "wrote", out, "for", sofullname;
}

END {
  if (! parse_names)
    emit(0);
  else if (soname != "") {
    emit(1);
  }
}
