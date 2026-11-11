#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_SPWENC_REFERENCE_ROOT:?Set to an extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_SPWENC_REFERENCE_ROOT:/firmware:ro" \
 "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
cd /source/vendor/gt-be98-pro/open-source/httpd-pwenc
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
$cc -std=gnu11 -Wall -Wextra -Werror -DREFERENCE -I/source/release/src/router/openssl-3.5/include test.c /source/release/src/router/httpd/prebuild/GT-BE98_PRO/pwenc.o -L/firmware/lib -L/firmware/usr/lib -Wl,-rpath-link,/firmware/lib:/firmware/usr/lib -lcrypto -o /tmp/reference
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/reference > /tmp/reference.log
$cc -std=gnu11 -Wall -Wextra -Werror -I/source/release/src/router/openssl-3.5/include test.c pwenc.c -L/firmware/lib -L/firmware/usr/lib -Wl,-rpath-link,/firmware/lib:/firmware/usr/lib -lcrypto -o /tmp/arm
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/arm > /tmp/arm.log
gcc -std=gnu11 -Wall -Wextra -Werror -fsanitize=address,undefined test.c pwenc.c -lcrypto -o /tmp/native
/tmp/native > /tmp/native.log
python3 - <<PY
from pathlib import Path
outputs=[[s for s in Path("/tmp/"+f+".log").read_text().splitlines() if s.startswith("CHECK ")] for f in ["reference","arm","native"]]
assert len(outputs[0])==20363
assert outputs[0]==outputs[1]==outputs[2]
print("HTTP helpers: 20363 cases match reference, ARM and sanitized native candidate")
PY
'
