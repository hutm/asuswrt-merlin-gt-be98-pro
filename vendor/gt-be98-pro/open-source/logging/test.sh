#!/usr/bin/env bash
# Run from the repository root; compilation and execution stay inside Docker.
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
tests="$root/vendor/gt-be98-pro/open-source/logging"
source_file="$root/release/src/router/libasuslog/source/libasuslog.c"
image=${ASUS_COMPONENT_IMAGE:-amng-component-test}
extra=()
reference_test=0
if [ -n "${ASUS_REFERENCE_LIBRARY:-}" ]; then
  extra=(-v "$ASUS_REFERENCE_LIBRARY:/reference.so:ro")
  reference_test=1
fi
docker run --rm --network none --user "$(id -u):$(id -g)" \
  -v "$tests:/tests:ro" -v "$source_file:/implementation.c:ro" \
  -v "$root/release/src/router/libasuslog/Makefile:/integration-Makefile:ro" \
  -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
  "${extra[@]}" -e "RUN_REFERENCE=$reference_test" "$image" bash -c '
set -euo pipefail
ulimit -c 0
cp -r /tests /tmp/source
cp /implementation.c /tmp/source/libasuslog.c
cp /integration-Makefile /tmp/source/integration-Makefile
for test in test_logging test_concurrency test_failures; do
  gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -pthread \
    /tmp/source/libasuslog.c /tmp/source/$test.c -o /tmp/$test
  timeout 30 /tmp/$test
  /toolchain/bin/arm-buildroot-linux-gnueabi-gcc -std=c99 -Wall -Wextra -Werror \
    -pthread /tmp/source/libasuslog.c /tmp/source/$test.c -o /tmp/$test-arm
  timeout 30 qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/$test-arm
done
mkdir /tmp/output
LOGGING_FIXTURE=/tmp/source LOGGING_OUTPUT=/tmp/output bash /tmp/source/test-integration.sh
if [ "$RUN_REFERENCE" = 1 ]; then
  LOGGING_FIXTURE=/tmp/source LOGGING_OUTPUT=/tmp/output \
    CANDIDATE_LIBRARY=/tmp/output/integration-libasuslog.so bash /tmp/source/run-compare.sh
fi
'
