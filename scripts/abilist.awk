# This awk script processes the output of objdump --dynamic-syms
# into a simple format that should not change when the ABI is not changing.

BEGIN {
  if (combine_fullname)
    combine = 1;
  if (combine)
    parse_names = 1;
}

# Per-file header.
/[^ :]+\.so\.[0-9.]+:[ 	]+.file format .*$/ {
  emit(0);

  seen_opd = 0;

  sofullname = $1;
  sub(/:$/, "", sofullname);
  soname = sofullname;
  sub(/^.*\//, "", soname);
  sub(/\.so\.[0-9.]+$/, "", soname);

  suppress = ((filename_regexp != "" && sofullname !~ filename_regexp) \
	      || (libname_regexp != "" && soname !~ libname_regexp));

  next
}

suppress { next }

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

  desc = "";
  if (type == "D" && $4 == ".tbss") {
    type = "T";
  }
  else if (type == "D" && $4 == ".opd") {
    type = "F";
    size = "";
    if (seen_opd < 0)
      type = "O";
    seen_opd = 1;
  }
  else if ($4 == "*ABS*") {
    type = "A";
    size = "";
  }
  else if (type == "DO") {
    type = "D";
  }
  else if (type == "DF") {
    if (symbol ~ /^\./ && seen_opd >= 0)
      next;
    seen_opd = -1;
    type = "F";
    size = "";
  }
  else {
    desc = symbol " " version " " weak " ? " type " " $4 " " $5;
  }
  if (size == " 0x") {
    desc = symbol " " version " " weak " ? " type " " $4 " " $5;
  }

  # Disabled -- weakness should not matter to shared library ABIs any more.
  #if (weak == "w") type = tolower(type);
  if (desc == "")
    desc = " " symbol " " type size;

  if (combine)
    version = soname " " version (combine_fullname ? " " sofullname : "");

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
  print "Don't grok this line:", $0
}

function emit(end) {
  if (!end && (combine || ! parse_names || soname == ""))
    return;
  tofile = parse_names && !combine;

  nverslist = 0;
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
    else {
      if (combine)
	print "";
      print prefix version;
      outpipe = "sort";
    }
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
  emit(1);
}
