#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_SPWENC_REFERENCE_ROOT:?Set to an extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
  -v "$root:/source:ro" -v "$ASUS_SPWENC_REFERENCE_ROOT:/firmware:ro" \
  -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
  "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
cd /source/vendor/gt-be98-pro/open-source/spwenc
compiler=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
gcc -DREAL_NAMES -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined names.c policy.c test_policy.c -lcrypto -o /tmp/native
/tmp/native >/tmp/native.txt
$compiler -DREAL_NAMES -std=c11 -Wall -Wextra -Werror -I/source/release/src/router/openssl-3.5/include names.c policy.c test_policy.c \
 -L/firmware/lib -L/firmware/usr/lib -Wl,-rpath-link,/firmware/lib:/firmware/usr/lib -lcrypto -o /tmp/arm
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/arm >/tmp/arm.txt
$compiler -DREAL_NAMES -std=c11 -Wall -Wextra -Werror -DREFERENCE -rdynamic test_policy.c -ldl -o /tmp/reference
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/reference >/tmp/reference.txt
cmp /tmp/native.txt /tmp/arm.txt
cmp /tmp/arm.txt /tmp/reference.txt
echo "Combined policy lookup: 15 values and 180 state/version cases match native/ARM/reference"
'
