#! /bin/sh

common_objpfx=$1; shift
elf_objpfx=$1; shift
rtld_installed_name=$1; shift
logfile=$common_objpfx/posix/globtest.out

# We have to make the paths `common_objpfx' absolute.
case "$common_objpfx" in
  .*)
    common_objpfx="`pwd`/$common_objpfx"
    ;;
  *)
    ;;
esac

# We have to find the libc and the NSS modules.
library_path=${common_objpfx}:${common_objpfx}nss:${common_objpfx}nis:${common_objpfx}db2:${common_objpfx}hesiod

# Since we use `sort' we must make sure to use the same locale everywhere.
LC_ALL=C
export LC_ALL
LANG=C
export LANG

# Create the arena
: ${TMPDIR=/tmp}
testdir=$TMPDIR/globtest-dir
testout=$TMPDIR/globtest-out

trap 'chmod 777 $testdir/noread; rm -fr $testdir $testout' 1 2 3 15

rm -fr $testdir 2>/dev/null
mkdir $testdir
echo 1 > $testdir/file1
echo 2 > $testdir/file2
echo 3 > $testdir/-file3
echo 4 > $testdir/~file4
echo 5 > $testdir/.file5
echo 6 > $testdir/'*file6'
mkdir $testdir/dir1
mkdir $testdir/dir2
test -d $testdir/noread || mkdir $testdir/noread
chmod a-r $testdir/noread
echo 1_1 > $testdir/dir1/file1_1
echo 1_2 > $testdir/dir1/file1_2

# Run some tests.
result=0
rm -f $logfile

# Normal test
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`*file6'
`-file3'
`dir1'
`dir2'
`file1'
`file2'
`noread'
`~file4'
EOF
if test $failed -ne 0; then
  echo "Normal test failed" >> $logfile
  result=1
fi

# Don't let glob sort it
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -s "$testdir" "*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`*file6'
`-file3'
`dir1'
`dir2'
`file1'
`file2'
`noread'
`~file4'
EOF
if test $failed -ne 0; then
  echo "No sort test failed" >> $logfile
  result=1
fi

# Mark directories
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -m "$testdir" "*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`*file6'
`-file3'
`dir1/'
`dir2/'
`file1'
`file2'
`noread/'
`~file4'
EOF
if test $failed -ne 0; then
  echo "Mark directories test failed" >> $logfile
  result=1
fi

# Find files starting with .
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -p "$testdir" "*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`*file6'
`-file3'
`.'
`..'
`.file5'
`dir1'
`dir2'
`file1'
`file2'
`noread'
`~file4'
EOF
if test $failed -ne 0; then
  echo "Leading period test failed" >> $logfile
  result=1
fi

# Test braces
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -b "$testdir" "file{1,2}" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`file1'
`file2'
EOF
if test $failed -ne 0; then
  echo "Braces test failed" >> $logfile
  result=1
fi

# Test NOCHECK
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -c "$testdir" "abc" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`abc'
EOF
if test $failed -ne 0; then
  echo "No check test failed" >> $logfile
  result=1
fi

# Test NOMAGIC without magic characters
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -g "$testdir" "abc" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`abc'
EOF
if test $failed -ne 0; then
  echo "No magic test failed" >> $logfile
  result=1
fi

# Test NOMAGIC with magic characters
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -g "$testdir" "abc*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "No magic w/ magic chars test failed" >> $logfile
  result=1
fi

# Test subdirs correctly
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`dir1/file1_1'
`dir1/file1_2'
EOF
if test $failed -ne 0; then
  echo "Subdirs test failed" >> $logfile
  result=1
fi

# Test subdirs for invalid names
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/1" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "Invalid subdir test failed" >> $logfile
  result=1
fi

# Test subdirs with wildcard
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/*1_1" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`dir1/file1_1'
EOF
if test $failed -ne 0; then
  echo "Wildcard subdir test failed" >> $logfile
  result=1
fi

# Test subdirs with ?
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/*?_?" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`dir1/file1_1'
`dir1/file1_2'
EOF
if test $failed -ne 0; then
  echo "Wildcard2 subdir test failed" >> $logfile
  result=1
fi

failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/file1_1" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`dir1/file1_1'
EOF
if test $failed -ne 0; then
  echo "Wildcard3 subdir test failed" >> $logfile
  result=1
fi

failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*-/*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "Wildcard4 subdir test failed" >> $logfile
  result=1
fi

failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*-" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "Wildcard5 subdir test failed" >> $logfile
  result=1
fi

# Test subdirs with ?
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/*?_?" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`dir1/file1_1'
`dir1/file1_2'
EOF
if test $failed -ne 0; then
  echo "Wildcard6 subdir test failed" >> $logfile
  result=1
fi

# Test subdirs with [ .. ]
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/file1_[12]" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`dir1/file1_1'
`dir1/file1_2'
EOF
if test $failed -ne 0; then
  echo "Brackets test failed" >> $logfile
  result=1
fi

# Test ']' inside bracket expression
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "dir1/file1_[]12]" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`dir1/file1_1'
`dir1/file1_2'
EOF
if test $failed -ne 0; then
  echo "Brackets2 test failed" >> $logfile
  result=1
fi

# Test tilde expansion
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -q -t "$testdir" "~" |
sort >$testout
echo ~ | cmp - $testout >> $logfile || failed=1
if test $failed -ne 0; then
  if test -d ~; then
    echo "Tilde test failed" >> $logfile
    result=1
  else
    echo "Tilde test could not be run" >> $logfile
  fi
fi

# Test tilde expansion with trailing slash
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -q -t "$testdir" "~/" |
sort > $testout
# Some shell incorrectly(?) convert ~/ into // if ~ expands to /.
if test ~/ = //; then
    echo / | cmp - $testout >> $logfile || failed=1
else
    echo ~/ | cmp - $testout >> $logfile || failed=1
fi
if test $failed -ne 0; then
  if test -d ~/; then
    echo "Tilde2 test failed" >> $logfile
    result=1
  else
    echo "Tilde2 test could not be run" >> $logfile
  fi
fi

# Test tilde expansion with username
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -q -t "$testdir" "~"$USER |
sort > $testout
eval echo ~$USER | cmp - $testout >> $logfile || failed=1
if test $failed -ne 0; then
  if eval test -d ~$USER; then
    echo "Tilde3 test failed" >> $logfile
    result=1
  else
    echo "Tilde3 test could not be run" >> $logfile
  fi
fi

# Tilde expansion shouldn't match a file
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -T "$testdir" "~file4" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "Tilde4 test failed" >> $logfile
  result=1
fi

# Matching \** should only find *file6
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "\**" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`*file6'
EOF
if test $failed -ne 0; then
  echo "Star test failed" >> $logfile
  result=1
fi

# ... unless NOESCAPE is used, in which case it shouldn't match anything.
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -e "$testdir" "\**" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "Star2 test failed" >> $logfile
  result=1
fi

# Matching \*file6 should find *file6
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "\*file6" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`*file6'
EOF
if test $failed -ne 0; then
  echo "Star3 test failed" >> $logfile
  result=1
fi

# Try a recursive failed search
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -e "$testdir" "a*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "Star4 test failed" >> $logfile
  result=1
fi

# ... with GLOB_ERR
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -E "$testdir" "a*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "Star5 test failed" >> $logfile
  result=1
fi

# Try a recursive search in unreadable directory
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "noread/*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "Star6 test failed" >> $logfile
  result=1
fi

failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "noread*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_NOMATCH
EOF
if test $failed -ne 0; then
  echo "Star6 test failed" >> $logfile
  result=1
fi

# The following tests will fail if run as root.
user=`id -un 2> /dev/null`
if test -z "$user"; then
    uid="$USER"
fi
if test "$user" != root; then
    # ... with GLOB_ERR
    ${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
    ${common_objpfx}posix/globtest -E "$testdir" "noread/*" |
    sort > $testout
    cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_ABORTED
EOF

    ${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
    ${common_objpfx}posix/globtest -E "$testdir" "noread*/*" |
    sort > $testout
    cat <<"EOF" | cmp - $testout >> $logfile || failed=1
GLOB_ABORTED
EOF
if test $failed -ne 0; then
  echo "GLOB_ERR test failed" >> $logfile
  result=1
fi
fi # not run as root

# Try multiple patterns (GLOB_APPEND)
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "file1" "*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`dir1/file1_1'
`dir1/file1_2'
`file1'
EOF
if test $failed -ne 0; then
  echo "GLOB_APPEND test failed" >> $logfile
  result=1
fi

# Try multiple patterns (GLOB_APPEND) with offset (GLOB_DOOFFS)
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -o "$testdir" "file1" "*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`abc'
`dir1/file1_1'
`dir1/file1_2'
`file1'
EOF
if test $failed -ne 0; then
  echo "GLOB_APPEND2 test failed" >> $logfile
  result=1
fi

# Test NOCHECK with non-existing file in subdir.
failed=0
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -c "$testdir" "*/blahblah" |
sort > $testout
cat <<"EOF" | cmp - $testout >> $logfile || failed=1
`dir1/blahblah'
`dir2/blahblah'
`noread/blahblah'
EOF
if test $failed -ne 0; then
  echo "No check2 test failed" >> $logfile
  result=1
fi

if test $result -eq 0; then
    chmod 777 $testdir/noread
    rm -fr $testdir $testout
    echo "All OK." > $logfile
fi

exit $result

# Preserve executable bits for this shell script.
Local Variables:
eval:(defun frobme () (set-file-modes buffer-file-name file-mode))
eval:(make-local-variable 'file-mode)
eval:(setq file-mode (file-modes (buffer-file-name)))
eval:(make-local-variable 'after-save-hook)
eval:(add-hook 'after-save-hook 'frobme)
End:
