#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
tests="$root/vendor/gt-be98-pro/open-source/nvram/compat"
extra=(); reference=0
if [ -n "${ASUS_REFERENCE_LIBRARY:-}" ]; then
  extra=(-v "$ASUS_REFERENCE_LIBRARY:/reference.so:ro"); reference=1
fi
docker run --rm --network none --user "$(id -u):$(id -g)" \
  -v "$tests:/tests:ro" \
  -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
  "${extra[@]}" -e "RUN_REFERENCE=$reference" \
  "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
set -euo pipefail
cd /tests
for name in bitflags strings numbers; do
  gcc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -fPIC -shared "$name.c" -o /tmp/native.so
  gcc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -rdynamic "test_$name.c" -ldl -o /tmp/test-native
  timeout 30 /tmp/test-native /tmp/native.so >"/tmp/$name-native.log"
  /toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
    -fPIC -shared "$name.c" -o /tmp/arm.so
  /toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
    -rdynamic "test_$name.c" -ldl -o /tmp/test-arm
  timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot \
    /tmp/test-arm /tmp/arm.so >"/tmp/$name-arm.log"
  cmp "/tmp/$name-native.log" "/tmp/$name-arm.log"
  if [ "$RUN_REFERENCE" = 1 ]; then
    timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot \
      /tmp/test-arm /reference.so >"/tmp/$name-reference.log" 2>"/tmp/$name-reference.stderr"
    cmp "/tmp/$name-arm.log" "/tmp/$name-reference.log"
  fi
done
python3 /tests/check-results.py /tmp
'
