#! /bin/sh
echo "static const uint32_t to_ucs4[256] = {"
sed -ne '/^[^[:space:]]*[[:space:]]*.x00/d' \
    -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/  [0x\1] = 0x\2,/p' \
    "$@" | sort -u
echo "};"
echo "static const struct gap from_idx[] = {"
sed -ne 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/\2/p' \
    "$@" | sort -u | $AWK -f gap.awk
echo "  { start: 0xffff, end: 0xffff, idx:     0 }"
echo "};"
echo "static const char from_ucs4[] = {"
sed -ne 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/\2 \1/p' \
    "$@" | sort -u | $AWK -f gaptab.awk
echo "};"
