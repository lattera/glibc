#! /bin/sh

common_objpfx=$1; shift
elf_objpfx=$1; shift
rtld_installed_name=$1; shift

# Create the arena
: ${TMPDIR=/tmp}
testdir=$TMPDIR/globtest-dir
testout=$TMPDIR/globtest-out

trap 'rm -fr $testdir $testout' 1 2 3 15

rm -fr $testdir
mkdir $testdir
echo 1 > $testdir/file1
echo 2 > $testdir/file2
mkdir $testdir/dir1
mkdir $testdir/dir2
echo 1_1 > $testdir/dir1/file1_1
echo 1_2 > $testdir/dir1/file1_2

# Run some tests.
result=0

${elf_objpfx}${rtld_installed_name} --library-path ${common_objpfx} \
${common_objpfx}posix/globtest "$testdir" "*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1'
`dir2'
`file1'
`file2'
not NULL
EOF

${elf_objpfx}${rtld_installed_name} --library-path $common_objpfx \
${common_objpfx}posix/globtest "$testdir" "*/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
`dir1/file1_2'
not NULL
EOF

${elf_objpfx}${rtld_installed_name} --library-path $common_objpfx \
${common_objpfx}posix/globtest "$testdir" "*/1" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
NULL
EOF

${elf_objpfx}${rtld_installed_name} --library-path $common_objpfx \
${common_objpfx}posix/globtest "$testdir" "*/*1_1" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
not NULL
EOF

${elf_objpfx}${rtld_installed_name} --library-path $common_objpfx \
${common_objpfx}posix/globtest "$testdir" "*/file1_1" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
`dir1/file1_1'
not NULL
EOF

${elf_objpfx}${rtld_installed_name} --library-path $common_objpfx \
${common_objpfx}posix/globtest "$testdir" "*-/*" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
NULL
EOF

${elf_objpfx}${rtld_installed_name} --library-path $common_objpfx \
${common_objpfx}posix/globtest "$testdir" "*-" |
sort > $testout
cat <<"EOF" | cmp - $testout || result=1
GLOB_NOMATCH
NULL
EOF

if test $result -eq 0; then
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
