#!/bin/bash
set -euo pipefail
ulimit -c 0
fixture=${LOGGING_FIXTURE:-/source}
output=${LOGGING_OUTPUT:-/output}
CC=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
SYSROOT=/toolchain/arm-buildroot-linux-gnueabi/sysroot
mkdir -p /tmp/stubs
$CC -fPIC -shared "$fixture"/reference_stubs.c -o /tmp/stubs/libnvram.so
$CC -fPIC -shared "$fixture"/reference_stubs.c -o /tmp/stubs/libshared.so
if [ -n "${CANDIDATE_LIBRARY:-}" ]; then
    cp "$CANDIDATE_LIBRARY" /tmp/candidate.so
else
    $CC -fPIC -shared -pthread "$fixture"/libasuslog.c -L/tmp/stubs -lnvram -o /tmp/candidate.so
fi
$CC -Wall -Wextra -Werror -Wl,--export-dynamic "$fixture"/compare_logging.c -ldl -o /tmp/compare
for label in reference candidate; do
    rm -rf /tmp/log-compare /tmp/asusdebuglog
    if [ "$label" = reference ]; then lib=/reference.so; else lib=/tmp/candidate.so; fi
    timeout 30 qemu-arm -L "$SYSROOT" -E LD_LIBRARY_PATH=/tmp/stubs -E TZ=UTC /tmp/compare "$lib" > "$output"/$label-output.txt
done
diff -u "$output"/reference-output.txt "$output"/candidate-output.txt
