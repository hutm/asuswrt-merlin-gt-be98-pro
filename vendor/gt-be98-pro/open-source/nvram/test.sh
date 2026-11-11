#!/usr/bin/env bash
# Component tests only; no firmware installation or persistent router writes.
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
tests="$root/vendor/gt-be98-pro/open-source/nvram"
image=${ASUS_COMPONENT_IMAGE:-amng-component-test}
docker run --rm --network none --user "$(id -u):$(id -g)" \
  -v "$tests:/tests:ro" -v "$root:/source:ro" \
  -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
  "$image" bash -ec '
set -euo pipefail
ulimit -c 0
cp -r /tests /tmp/source
cp /source/release/src/router/shared/source/nvpriv.c /tmp/source/storage.c
cp /source/release/src/router/shared/source/storage_keys.h /tmp/source/storage_keys.h
python3 /tmp/source/make-reference.py /source /tmp/source/reference.h
cd /tmp/source
for name in pair frame getall settings persist client cache notification listener transport client_transport; do
  gcc -std=c11 -Wall -Wextra -Werror -pthread -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    "test_$name.c" -o "/tmp/test_$name"
  timeout 30 "/tmp/test_$name"
  /toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 \
    -Wall -Wextra -Werror -pthread "test_$name.c" -o "/tmp/test_$name-arm"
  case "$name" in
    transport|client_transport)
      echo "$name: ARM compilation passed; Netlink runtime requires native Linux" ;;
    *) timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot "/tmp/test_$name-arm" ;;
  esac
done
mkdir /tmp/core-native /tmp/core-arm
gcc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
  -pthread -fPIC -shared core.c adapter.c -o /tmp/core-native/libnvcore.so
gcc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
  -pthread test_core.c -L/tmp/core-native -Wl,-rpath,/tmp/core-native -lnvcore -o /tmp/test-core
timeout 30 /tmp/test-core
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
  -pthread -fPIC -shared core.c adapter.c -o /tmp/core-arm/libnvcore.so
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
  -pthread test_core.c -L/tmp/core-arm -Wl,-rpath,/tmp/core-arm -lnvcore -o /tmp/test-core-arm
timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/test-core-arm
gcc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
  -pthread test_adapter.c -L/tmp/core-native -Wl,-rpath,/tmp/core-native -lnvcore -o /tmp/test-adapter
timeout 30 /tmp/test-adapter
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
  -pthread test_adapter.c -L/tmp/core-arm -Wl,-rpath,/tmp/core-arm -lnvcore -o /tmp/test-adapter-arm
timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/test-adapter-arm
for name in storage storage_bulk; do
  gcc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
    storage.c "test_$name.c" -o /tmp/test-storage
  timeout 30 /tmp/test-storage
  /toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
    storage.c "test_$name.c" -o /tmp/test-storage-arm
  timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/test-storage-arm
done
'
bash "$tests/compat/test.sh"
