#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_REFERENCE_ROOT:?Set to an extracted reference firmware root}"
: "${ASUS_CANDIDATE_ROOT:?Set to the extracted candidate firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_REFERENCE_ROOT:/reference:ro" -v "$ASUS_CANDIDATE_ROOT:/candidate:ro" \
 "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
suite=/source/vendor/gt-be98-pro/open-source/notify-rc/test.c
$cc -O2 -Wall -Wextra -Werror -DLINKED_SHARED -rdynamic "$suite" -ldl -o /tmp/reference
$cc -O2 -Wall -Wextra -Werror -DLINKED_SHARED -DCANDIDATE_ONLY -rdynamic "$suite" -ldl -o /tmp/candidate
for variant in reference candidate; do
 qemu-arm -L /$variant -E LD_LIBRARY_PATH=/$variant/lib:/$variant/usr/lib /tmp/$variant > /tmp/$variant.log
done
cmp /tmp/reference.log /tmp/candidate.log
'
