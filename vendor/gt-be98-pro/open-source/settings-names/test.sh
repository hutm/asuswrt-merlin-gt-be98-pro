#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_REFERENCE_ROOT:?Set to an extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_REFERENCE_ROOT:/firmware:ro" "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
cd /source/vendor/gt-be98-pro/open-source/settings-names
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
$cc -std=gnu11 -Wall -Wextra -Werror test.c /source/release/src/router/shared/prebuild/GT-BE98_PRO/shutils_private.o -Wl,--wrap=strdup -o /tmp/reference
qemu-arm -L /firmware /tmp/reference > /tmp/reference.log
$cc -std=gnu11 -Wall -Wextra -Werror test.c settings_names.c -Wl,--wrap=strdup -o /tmp/arm
qemu-arm -L /firmware /tmp/arm > /tmp/arm.log
gcc -std=gnu11 -Wall -Wextra -Werror -fsanitize=address,undefined test.c settings_names.c -Wl,--wrap=strdup -o /tmp/native
/tmp/native > /tmp/native.log
cmp /tmp/reference.log /tmp/arm.log
cmp /tmp/arm.log /tmp/native.log
tail -1 /tmp/native.log
'
