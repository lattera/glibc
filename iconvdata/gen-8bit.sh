#! /bin/sh
echo "static const uint32_t to_ucs4[256] = {"
sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' \
    -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/  [0x\1] = 0x\2,/p' \
    -e d "$@" | \
sort -u
echo "};"
echo "static const char from_ucs4[] = {"
sed -e '/^[^[:space:]]*[[:space:]]*.x00/d' \
    -e 's/^[^[:space:]]*[[:space:]]*.x\(..\)[[:space:]]*<U\(....\)>.*/  [0x\2] = 0x\1,/p' \
    -e d "$@" | \
sort -u
echo "};"
