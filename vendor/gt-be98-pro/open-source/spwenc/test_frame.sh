#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_SPWENC_REFERENCE_ROOT:?Set to an extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
  -v "$root:/source:ro" \
  -v "$ASUS_SPWENC_REFERENCE_ROOT:/firmware:ro" \
  -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
  "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
cd /source/vendor/gt-be98-pro/open-source/spwenc
compiler=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
gcc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -I/source/release/src/router/zlib frame.c lengths.c test_frame.c -l:libz.so.1 -o /tmp/native
/tmp/native >/tmp/native.txt
$compiler -std=c11 -Wall -Wextra -Werror -I/source/release/src/router/zlib frame.c lengths.c test_frame.c -L/firmware/lib -L/firmware/usr/lib -Wl,-rpath-link,/firmware/lib:/firmware/usr/lib -l:libz.so.1 -o /tmp/arm
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/arm >/tmp/arm.txt
$compiler -std=c11 -Wall -Wextra -Werror -rdynamic reference_frame.c -ldl -o /tmp/reference
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/reference >/tmp/reference.txt
python3 - <<"PYTHON"
from pathlib import Path
import base64,zlib
native=Path("/tmp/native.txt").read_text().splitlines()
arm=Path("/tmp/arm.txt").read_text().splitlines()
reference=Path("/tmp/reference.txt").read_text().splitlines()
frames=[s for s in reference if s.startswith("FRAME ")]
assert len(frames)==64 and native==arm==frames
for n,line in enumerate(frames,1):
    payload=bytes.fromhex(line.split()[2]);text=b"a"*n
    assert payload[:8]==f"{zlib.crc32(text):08x}".encode()
    assert payload[8:8+n]==text and not any(payload[8+n:])
results=[s.split(" ",3) for s in reference if s.startswith("RESULT ")]
assert len(results)==65 and results[0]==["RESULT","0","0",""]
for n,row in enumerate(results[1:],1):
    assert row[2]=="0" and base64.b64decode(row[3])==bytes.fromhex(frames[n-1].split()[2])
decoded=[s.split(" ",3) for s in reference if s.startswith("DECODE ")]
assert len(decoded)==64
for n,row in enumerate(decoded,1):
    assert row==["DECODE",str(n),"1","a"*n],row
corrupted=[s.split() for s in reference if s.startswith("CORRUPT ")]
assert len(corrupted)==64 and all(row[2:]==["0","1"] for row in corrupted)
print("Frame decoding: valid reference records match; reference CRC failures pass through input (source frame parser rejects)")
print("Password framing: native/ARM/reference CRC, padding, Base64 and empty behavior match; capacity failures preserve output")
PYTHON
'
