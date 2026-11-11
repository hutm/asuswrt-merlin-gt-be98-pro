#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
p=/source/vendor/gt-be98-pro/open-source/spwenc
compiler=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
make -s -j2 -f "$p/Makefile" CC="$compiler" BUILD_DIR=/tmp/objects OUTPUT=/tmp/open-spwenc.o \
 CFLAGS="-Os -Wall -Wextra -Werror" CPPFLAGS="-I/source/release/src/router/openssl-3.5/include -I/source/release/src/router/zlib"
nm=/toolchain/bin/arm-buildroot-linux-gnueabi-nm
$nm -g --defined-only /source/release/src/router/shared/prebuild/GT-BE98_PRO/spwenc.o | awk "{print \$3}" | sort > /tmp/required
$nm -g --defined-only /tmp/open-spwenc.o | awk "{print \$3}" | sort > /tmp/provided
comm -23 /tmp/required /tmp/provided > /tmp/missing
test ! -s /tmp/missing || { cat /tmp/missing; exit 1; }
for object in /source/release/src/router/shared/prebuild/GT-BE98_PRO/spwenc.o /tmp/open-spwenc.o; do
 $nm -g -S --defined-only "$object" | awk "\$4 == \"ASUS_PP_t\" || \$4 == \"invalid_nvram_get_t\" {print \$4, \$2, \$3}" | sort
done > /tmp/tables
head -2 /tmp/tables > /tmp/original
tail -2 /tmp/tables > /tmp/source
cmp /tmp/original /tmp/source
printf "ARM combined object supplies all %s reference exports; exported table sizes and types match\n" "$(wc -l </tmp/required)"
'
