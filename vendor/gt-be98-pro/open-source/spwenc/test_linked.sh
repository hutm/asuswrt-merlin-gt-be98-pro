#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_SPWENC_REFERENCE_ROOT:?Set to the original extracted firmware root}"
: "${ASUS_SPWENC_CANDIDATE_DIR:?Set to a directory containing the new libshared.so}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_SPWENC_REFERENCE_ROOT:/firmware:ro" -v "$ASUS_SPWENC_CANDIDATE_DIR:/candidate:ro" \
 "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
cd /source/vendor/gt-be98-pro/open-source/spwenc
for suite in lifecycle wrapper helpers settings migration init access policy names timestamps policy_init md5_abi aes_abi commands; do
 /toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror -DREFERENCE -rdynamic "test_$suite.c" -ldl -o /tmp/test
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test >/tmp/original
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/candidate:/firmware/lib:/firmware/usr/lib /tmp/test >/tmp/candidate
 cmp /tmp/original /tmp/candidate
 echo "$suite: linked shared-library comparison passed"
done
'
