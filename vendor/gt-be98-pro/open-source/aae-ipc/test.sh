#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_REFERENCE_ROOT:?Set to an extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_REFERENCE_ROOT:/firmware:ro" "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
cd /source/vendor/gt-be98-pro/open-source/aae-ipc
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
nm=/toolchain/bin/arm-buildroot-linux-gnueabi-nm
headers=-I/source/release/src/router/shared
reference=/source/release/src/router/aaews/prebuild/GT-BE98_PRO/aae_ipc.o
wrap=();for name in socket fcntl connect getsockopt send select poll __poll_chk clock_gettime read __read_chk close printf puts;do wrap+=("-Wl,--wrap=$name");done
$cc -O2 -Wall -Wextra -Werror -fPIC "$headers" -c ipc.c -o /tmp/ipc.o
for variant in reference candidate; do
 object=$reference; [ "$variant" != candidate ] || object=/tmp/ipc.o
 "$nm" --defined-only --extern-only "$object" | awk "{print \$2, \$3}" | sort > /tmp/$variant.exports
done
diff -u /tmp/reference.exports /tmp/candidate.exports
$cc -O2 -Wall -Wextra -Werror test_protocol.c "$reference" "${wrap[@]}" -o /tmp/reference
qemu-arm -L /firmware /tmp/reference > /tmp/reference.log
$cc -O2 -Wall -Wextra -Werror -DCANDIDATE_ONLY test_protocol.c /tmp/ipc.o "${wrap[@]}" -o /tmp/candidate
qemu-arm -L /firmware /tmp/candidate > /tmp/candidate.log
cmp /tmp/reference.log /tmp/candidate.log
gcc -O2 -Wall -Wextra -Werror -DCANDIDATE_ONLY -DOPEN_IPC_HOST_TEST -fsanitize=address,undefined "$headers" test_protocol.c ipc.c "${wrap[@]}" -o /tmp/native
/tmp/native > /tmp/native.log
cmp /tmp/native.log /tmp/candidate.log
$cc -O2 -Wall -Wextra -Werror test_socket.c /tmp/ipc.o -o /tmp/socket-arm
qemu-arm -L /firmware /tmp/socket-arm
gcc -O2 -Wall -Wextra -Werror -DOPEN_IPC_HOST_TEST -fsanitize=address,undefined "$headers" test_socket.c ipc.c -o /tmp/socket-native
/tmp/socket-native
'
