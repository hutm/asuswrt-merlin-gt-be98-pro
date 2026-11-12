#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_REFERENCE_ROOT:?Set to extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_REFERENCE_ROOT:/firmware:ro" "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
suite=/source/vendor/gt-be98-pro/open-source/sw-auth
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
reference=/source/release/src/router/sw-hw-auth/prebuild/GT-BE98_PRO/sw_auth.o
includes="-I/source/release/src/router/openssl-3.5/include -I/source/release/src/router/sw-hw-auth"
$cc -O2 -Wall -Wextra -Werror -fPIC $includes -c "$suite/auth_code.c" -o /tmp/candidate.o
for variant in reference candidate; do
 object=$reference; [ "$variant" != candidate ] || object=/tmp/candidate.o
 /toolchain/bin/arm-buildroot-linux-gnueabi-nm --defined-only --extern-only "$object" | awk "{print \$2, \$3}" | sort > /tmp/$variant.exports
 $cc -O2 -Wall -Wextra -Werror "$suite/test.c" "$object" -L/source/release/src/router/openssl-3.5 -lcrypto -o /tmp/$variant
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/$variant > /tmp/$variant.log
done
diff -u /tmp/reference.exports /tmp/candidate.exports
cmp /tmp/reference.log /tmp/candidate.log
for test in test test_invalid test_hash_failure; do
 wrap=""; [ "$test" != test_hash_failure ] || wrap="-Wl,--wrap=SHA256"
 gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -I/source/release/src/router/sw-hw-auth "$suite/$test.c" "$suite/auth_code.c" $wrap -lcrypto -o /tmp/native
 /tmp/native > /tmp/$test.log
 if [ "$test" != test ]; then
  $cc -O2 -Wall -Wextra -Werror $includes "$suite/$test.c" /tmp/candidate.o $wrap -L/source/release/src/router/openssl-3.5 -lcrypto -o /tmp/arm
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/arm
 fi
done
cmp /tmp/reference.log /tmp/test.log
grep -Ec "^(CASE|LENGTH|VALUE|RANDOM) " /tmp/candidate.log
'
