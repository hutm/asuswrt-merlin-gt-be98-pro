#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_REFERENCE_ROOT:?Set to extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_REFERENCE_ROOT:/firmware:ro" "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
suite=/source/vendor/gt-be98-pro/open-source/amas-dwb
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
reference=/source/release/src/router/shared/prebuild/GT-BE98_PRO/amas_dwb.o
$cc -O2 -Wall -Wextra -Werror -fPIC -I/source/release/src/router/shared -c "$suite/settings.c" -o /tmp/candidate.o
for variant in reference candidate; do
 object=$reference; [ "$variant" != candidate ] || object=/tmp/candidate.o
 /toolchain/bin/arm-buildroot-linux-gnueabi-nm --defined-only --extern-only "$object" | awk "{print \$2, \$3}" | sort > /tmp/$variant.exports
 $cc -O2 -Wall -Wextra -Werror "$suite/test.c" "$object" -o /tmp/$variant
 qemu-arm -L /firmware /tmp/$variant > /tmp/$variant.log
done
diff -u /tmp/reference.exports /tmp/candidate.exports
cmp /tmp/reference.log /tmp/candidate.log
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -I/source/release/src/router/shared "$suite/test.c" "$suite/settings.c" -o /tmp/native
/tmp/native > /tmp/native.log
cmp /tmp/reference.log /tmp/native.log
grep -c "^CASE " /tmp/candidate.log
'
