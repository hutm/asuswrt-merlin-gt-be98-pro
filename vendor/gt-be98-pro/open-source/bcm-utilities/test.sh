#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_REFERENCE_ROOT:?Set to an extracted reference firmware root}"
: "${ASUS_BCM_OBJECT_DIR:?Set to a directory containing the three source-built objects}"
suite=$(cd -- "$(dirname -- "$0")" && pwd)
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_REFERENCE_ROOT:/firmware:ro" -v "$ASUS_BCM_OBJECT_DIR:/candidate:ro" \
 -v "$suite:/suite:ro" "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
wl=/source/release/src-rt-5.04behnd.4916/bcmdrivers/broadcom/net/wl/bcm96813/main/src
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
nm=/toolchain/bin/arm-buildroot-linux-gnueabi-nm
reference=/source/release/src/router/shared/prebuild/GT-BE98_PRO
includes=(-I"$wl/include" -I"$wl/common/include" -I"$wl/shared/bcmwifi/include" -I"$wl/../components/math/include" -I"$wl/../components/proto/include")
for name in bcmutils bcmxtlv bcmwifi_channels; do
 for variant in reference candidate; do
  directory=$reference; [ "$variant" != candidate ] || directory=/candidate
  "$nm" --defined-only --extern-only "$directory/$name.o" | awk "{print \$2, \$3}" | sort > "/tmp/$name-$variant.exports"
 done
 diff -u "/tmp/$name-reference.exports" "/tmp/$name-candidate.exports"
done
for test in utils xtlv channels; do
 case "$test" in
  utils) names=(bcmutils); sources=("$wl/shared/bcmutils.c");;
  xtlv) names=(bcmxtlv); sources=("$wl/shared/bcmxtlv.c");;
  channels) names=(bcmutils bcmwifi_channels); sources=("$wl/shared/bcmutils.c" "$wl/shared/bcmwifi/src/bcmwifi_channels.c");;
 esac
 for variant in reference candidate; do
  directory=$reference; [ "$variant" != candidate ] || directory=/candidate
  objects=(); for name in "${names[@]}"; do objects+=("$directory/$name.o"); done
  "$cc" -std=gnu11 -Wall -Wextra -Werror "${includes[@]}" "/suite/test_$test.c" "${objects[@]}" -o "/tmp/$test-$variant"
  qemu-arm -L /firmware "/tmp/$test-$variant" > "/tmp/$test-$variant.log"
 done
 gcc -std=gnu11 -Ulinux -Wall -fsanitize=address,undefined "${includes[@]}" "/suite/test_$test.c" "${sources[@]}" -o "/tmp/$test-native"
 "/tmp/$test-native" > "/tmp/$test-native.log"
 cmp "/tmp/$test-reference.log" "/tmp/$test-candidate.log"
 cmp "/tmp/$test-candidate.log" "/tmp/$test-native.log"
 printf "%s: %s matching records\n" "$test" "$(wc -l < "/tmp/$test-native.log")"
done
'
