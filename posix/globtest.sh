#! /bin/sh

common_objpfx=$1; shift
elf_objpfx=$1; shift
rtld_installed_name=$1; shift

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

# Normal test
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`*file6'
`-file3'
`dir1'
`dir2'
`file1'
`file2'
`noread'
`~file4'
EOF

# Don't let glob sort it
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -s "$testdir" "*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`*file6'
`-file3'
`dir1'
`dir2'
`file1'
`file2'
`noread'
`~file4'
EOF

# Mark directories
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -m "$testdir" "*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`*file6'
`-file3'
`dir1/'
`dir2/'
`file1'
`file2'
`noread/'
`~file4'
EOF

# Find files starting with .
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -p "$testdir" "*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
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

# Test braces
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -b "$testdir" "file{1,2}" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`file1'
`file2'
EOF

# Test NOCHECK
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -c "$testdir" "abc" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`abc'
EOF

# Test NOMAGIC without magic characters
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -g "$testdir" "abc" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`abc'
EOF

# Test NOMAGIC with magic characters
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -g "$testdir" "abc*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

# Test subdirs correctly
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
`dir1/file1_2'
EOF

# Test subdirs for invalid names
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/1" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

# Test subdirs with wildcard
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/*1_1" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
EOF

# Test subdirs with ?
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/*?_?" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
`dir1/file1_2'
EOF

${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/file1_1" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
EOF

${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*-/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*-" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

# Test subdirs with ?
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/*?_?" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
`dir1/file1_2'
EOF

# Test subdirs with [ .. ]
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "*/file1_[12]" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
`dir1/file1_2'
EOF

# Test ']' inside bracket expression
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "dir1/file1_[]12]" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
`dir1/file1_2'
EOF

# Test tilde expansion
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -q -t "$testdir" "~" |
sort >$testout
echo ~ | cmp - $testout || result=1

# Test tilde expansion with trailing slash
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -q -t "$testdir" "~/" |
sort > $testout
echo ~/ | cmp - $testout || result=1

# Test tilde expansion with username
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -q -t "$testdir" "~"$USER |
sort > $testout
# Some shell incorrectly(?) convert ~/ into // if ~ expands to /.
if test ~/ = //; then
    echo / | cmp - $testout || result=1
else
    echo ~/ | cmp - $testout || result=1
fi

# Tilde expansion shouldn't match a file
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -T "$testdir" "~file4" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

# Matching \** should only find *file6
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "\**" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`*file6'
EOF

# ... unless NOESCAPE is used, in which case it shouldn't match anything.
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -e "$testdir" "\**" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

# Matching \*file6 should find *file6
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "\*file6" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`*file6'
EOF

# Try a recursive failed search
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -e "$testdir" "a*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

# ... with GLOB_ERR
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -E "$testdir" "a*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

# Try a recursive search in unreadable directory
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "noread/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "noread*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
EOF

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
    cat <<"EOF" | cmp - $testout || result=1
GLOB_ABORTED
EOF

    ${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
    ${common_objpfx}posix/globtest -E "$testdir" "noread*/*" |
    sort > $testout
    cat <<"EOF" | cmp - $testout || result=1
GLOB_ABORTED
EOF
fi # not run as root

# Try multiple patterns (GLOB_APPEND)
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest "$testdir" "file1" "*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
`dir1/file1_2'
`file1'
EOF

# Try multiple patterns (GLOB_APPEND) with offset (GLOB_DOOFFS)
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -o "$testdir" "file1" "*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`abc'
`dir1/file1_1'
`dir1/file1_2'
`file1'
EOF

# Test NOCHECK with non-existing file in subdir.
${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
${common_objpfx}posix/globtest -c "$testdir" "*/blahblah" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/blahblah'
`dir2/blahblah'
`noread/blahblah'
EOF

if test $result -eq 0; then
    chmod 777 $testdir/noread
    rm -fr $testdir $testout
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
