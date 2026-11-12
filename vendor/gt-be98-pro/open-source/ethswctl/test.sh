#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_REFERENCE_ROOT:?Set to extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_REFERENCE_ROOT:/firmware:ro" "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
suite=/source/vendor/gt-be98-pro/open-source/ethswctl
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
reference=/source/release/src/router/shared/prebuild/GT-BE98_PRO/ethswctl.o
$cc -O2 -Wall -Wextra -Werror -fPIC -c "$suite/jumbo_status.c" -o /tmp/candidate.o
for variant in reference candidate; do
 object=$reference; [ "$variant" != candidate ] || object=/tmp/candidate.o
 /toolchain/bin/arm-buildroot-linux-gnueabi-nm --defined-only --extern-only "$object" | awk "{print \$2, \$3}" | sort > /tmp/$variant.exports
 $cc -O2 -Wall -Wextra -Werror "$suite/test.c" "$object" -o /tmp/$variant
 qemu-arm -L /firmware /tmp/$variant > /tmp/$variant.log
done
diff -u /tmp/reference.exports /tmp/candidate.exports
cmp /tmp/reference.log /tmp/candidate.log
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined "$suite/test.c" "$suite/jumbo_status.c" -o /tmp/native
/tmp/native > /tmp/native.log
# libc puts return values need only be nonnegative on success; compare native text.
sed "/^RETURN /d" /tmp/reference.log > /tmp/reference.text
sed "/^RETURN /d" /tmp/native.log > /tmp/native.text
cmp /tmp/reference.text /tmp/native.text
grep -c "^RETURN " /tmp/candidate.log
'
