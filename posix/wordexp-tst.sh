#!/bin/sh

# Some of these tests may look a little weird.
# The first parameter to wordexp-test is what it gives to wordexp.
# The others are just there to be parameters.

common_objpfx=$1; shift
elf_objpfx=$1; shift
rtld_installed_name=$1; shift

: ${TMPDIR=/tmp}
testout=$TMPDIR/wordexp-test-result

failed=0
export IFS=$(echo -e " \t\n")

${elf_objpfx}${rtld_installed_name} --library-path ${common_objpfx} \
${common_objpfx}posix/wordexp-test '$*' > ${testout}1
cat <<"EOF" | cmp - ${testout}1 || failed=1
wordexp returned 0
we_wordv[0] = "$*"
EOF

${elf_objpfx}${rtld_installed_name} --library-path ${common_objpfx} \
${common_objpfx}posix/wordexp-test '${*}' unquoted > ${testout}2
cat <<"EOF" | cmp - ${testout}2 || failed=1
wordexp returned 0
we_wordv[0] = "${*}"
we_wordv[1] = "unquoted"
EOF

${elf_objpfx}${rtld_installed_name} --library-path ${common_objpfx} \
${common_objpfx}posix/wordexp-test '$@' unquoted > ${testout}3
cat <<"EOF" | cmp - ${testout}3 || failed=1
wordexp returned 0
we_wordv[0] = "$@"
we_wordv[1] = "unquoted"
EOF

${elf_objpfx}${rtld_installed_name} --library-path ${common_objpfx} \
${common_objpfx}posix/wordexp-test '"$* quoted"' param > ${testout}4
cat <<"EOF" | cmp - ${testout}4 || failed=1
wordexp returned 0
we_wordv[0] = ""$* quoted" param quoted"
EOF

${elf_objpfx}${rtld_installed_name} --library-path ${common_objpfx} \
${common_objpfx}posix/wordexp-test '"$@ quoted"' param > ${testout}5
cat <<"EOF" | cmp - ${testout}5 || failed=1
wordexp returned 0
we_wordv[0] = ""$@ quoted""
we_wordv[1] = "param quoted"
EOF
# Why?  Because bash does it that way..

${elf_objpfx}${rtld_installed_name} --library-path ${common_objpfx} \
${common_objpfx}posix/wordexp-test '$#' 2 3 4 5 > ${testout}6
cat <<"EOF" | cmp - ${testout}6 || failed=1
wordexp returned 0
we_wordv[0] = "5"
EOF

${elf_objpfx}${rtld_installed_name} --library-path ${common_objpfx} \
${common_objpfx}posix/wordexp-test '$2 ${3} $4' 2nd 3rd "4 th" > ${testout}7
cat <<"EOF" | cmp - ${testout}7 || failed=1
wordexp returned 0
we_wordv[0] = "2nd"
we_wordv[1] = "3rd"
we_wordv[2] = "4"
we_wordv[3] = "th"
EOF

exit $failed
