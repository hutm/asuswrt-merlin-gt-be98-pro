#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_CANDIDATE_ROOT:?Set to the extracted candidate firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_CANDIDATE_ROOT:/firmware:ro" "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -O2 -Wall -Wextra -Werror -DLINKED_SHARED \
 /source/vendor/gt-be98-pro/open-source/aae-ipc/test_socket.c -ldl -o /tmp/test
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test
'
