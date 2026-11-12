#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
suite=$(cd -- "$(dirname -- "$0")" && pwd)
: "${ASUS_REFERENCE_ROOT:?Set to an extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$suite:/suite:ro" -v "$ASUS_REFERENCE_ROOT:/firmware:ro" \
 "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
nm=/toolchain/bin/arm-buildroot-linux-gnueabi-nm
reference=/source/release/src/router/shared/prebuild/GT-BE98_PRO/uu_utils.o
$cc -O2 -Wall -Wextra -Werror -fPIC -I/source/release/src/router/shared -c /suite/model_check.c -o /tmp/model.o
for variant in reference candidate; do
 object=$reference; [ "$variant" != candidate ] || object=/tmp/model.o
 "$nm" --defined-only --extern-only "$object" | awk "{print \$2, \$3}" | sort > /tmp/$variant.exports
done
diff -u /tmp/reference.exports /tmp/candidate.exports
$cc -O2 -Wall -Wextra -Werror /suite/test.c "$reference" -o /tmp/reference
qemu-arm -L /firmware /tmp/reference > /tmp/reference.log
$cc -O2 -Wall -Wextra -Werror /suite/test.c /tmp/model.o -o /tmp/candidate
qemu-arm -L /firmware /tmp/candidate > /tmp/candidate.log
cmp /tmp/reference.log /tmp/candidate.log
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -I/source/release/src/router/shared /suite/test.c /suite/model_check.c -o /tmp/native
/tmp/native > /tmp/native.log
cmp /tmp/native.log /tmp/candidate.log
'
