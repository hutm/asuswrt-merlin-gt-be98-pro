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
python3 /tmp/source/make-reference.py /source /tmp/source/reference.h
cd /tmp/source
for name in pair frame getall settings persist client cache transport client_transport; do
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
'
