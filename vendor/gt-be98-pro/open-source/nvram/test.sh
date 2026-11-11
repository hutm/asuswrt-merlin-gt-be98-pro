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
for name in manager xfr pair frame getall settings persist client cache notification listener transport client_transport; do
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
gcc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  storage_lock.c test_storage_lock.c -o /tmp/test-storage-lock
timeout 30 /tmp/test-storage-lock
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
  storage_lock.c test_storage_lock.c -o /tmp/test-storage-lock-arm
timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/test-storage-lock-arm
gcc -std=c11 -Wall -Wextra -Werror -pthread -fsanitize=address,undefined \
  storage_binding.c storage_lock.c test_storage_binding.c -o /tmp/test-storage-binding
timeout 30 /tmp/test-storage-binding
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror -pthread \
  storage_binding.c storage_lock.c test_storage_binding.c -o /tmp/test-storage-binding-arm
timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/test-storage-binding-arm
gcc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  profile.c test_profile.c -o /tmp/test-profile
timeout 30 /tmp/test-profile
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
  profile.c test_profile.c -o /tmp/test-profile-arm
timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/test-profile-arm
gcc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  manufacturing.c test_manufacturing.c -o /tmp/test-manufacturing
timeout 30 /tmp/test-manufacturing
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
  manufacturing.c test_manufacturing.c -o /tmp/test-manufacturing-arm
timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/test-manufacturing-arm
gcc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  cli_kernel.c manufacturing.c test_cli_kernel.c -o /tmp/test-cli-kernel
timeout 30 /tmp/test-cli-kernel
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
  cli_kernel.c manufacturing.c test_cli_kernel.c -o /tmp/test-cli-kernel-arm
timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/test-cli-kernel-arm
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
runtime_sources="core.c adapter.c storage_binding.c storage_lock.c runtime.c compat/bitflags.c compat/strings.c compat/numbers.c"
gcc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined -DNV_RUNTIME_TEST \
  -pthread -fPIC -shared $runtime_sources -o /tmp/core-native/libnvruntime.so
gcc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined -rdynamic \
  -pthread test_runtime.c -L/tmp/core-native -Wl,-rpath,/tmp/core-native -lnvruntime -o /tmp/test-runtime
timeout 30 /tmp/test-runtime
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror -DNV_RUNTIME_TEST \
  -pthread -fPIC -shared $runtime_sources -o /tmp/core-arm/libnvruntime.so
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror -rdynamic \
  -pthread test_runtime.c -L/tmp/core-arm -Wl,-rpath,/tmp/core-arm -lnvruntime -o /tmp/test-runtime-arm
timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/test-runtime-arm
# Also compile the real transport path; it is never executed on the host.
/toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c11 -Wall -Wextra -Werror \
  -pthread -fPIC -shared $runtime_sources -o /tmp/core-arm/libnvruntime-production.so
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
