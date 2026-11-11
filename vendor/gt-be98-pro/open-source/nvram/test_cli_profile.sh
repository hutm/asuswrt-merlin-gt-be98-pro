#!/usr/bin/env bash
# Isolated ARM comparisons against an extracted reference firmware.
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_CLI_REFERENCE_ROOT:?Set to an extracted firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
  -v "$root:/source:ro" -v "$ASUS_CLI_REFERENCE_ROOT:/firmware:ro" \
  -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
  "${ASUS_COMPONENT_IMAGE:-amng-component-test}" bash -ec '
component=/source/vendor/gt-be98-pro/open-source/nvram
compiler=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
$compiler -std=gnu99 -Os -DASUS_OPEN_NVRAM_CLI -DHND_ROUTER \
  -I"$component" -I/source/release/src/router/shared \
  -I/source/release/src-rt-5.04behnd.4916/bcmdrivers/broadcom/net/wl/impl103/main/src/include \
  /source/release/src/router/nvram/nvram.c "$component/cli_kernel.c" "$component/cli_flags.c" \
  "$component/manufacturing.c" "$component/profile.c" \
  -L/firmware/lib -L/firmware/usr/lib -Wl,-rpath-link,/firmware/lib:/firmware/usr/lib \
  -lnvram -lshared -o /tmp/source-cli
$compiler -shared -fPIC "$component/test_cli_profile_stubs.c" -ldl -o /tmp/stubs.so
python3 - <<"PYTHON"
from pathlib import Path
import struct
p=b"lan_ipaddr=192.0.2.1\0wl_test=value\0wl0_failed=skip\0wgc1_test=keep\0test_exact=keep\0unknown=skip\0\0"
Path("/tmp/HDR1").write_bytes(b"HDR1"+struct.pack("<I",len(p))+p)
Path("/tmp/HDR2").write_bytes(b"HDR2"+struct.pack("<I",len(p))[:3]+bytes([7])+bytes((255-c+7)&255 if c else 254 for c in p))
Path("/tmp/expected").write_text("ASET\nSET[lan_ipaddr]=[192.0.2.1]\nASET\nSET[wl_test]=[value]\nASET\nSET[wgc1_test]=[keep]\nASET\nSET[test_exact]=[keep]\n")
PYTHON
for format in HDR1 HDR2; do
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /tmp/source-cli restore /tmp/$format >/tmp/source.log
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /firmware/bin/nvram restore /tmp/$format >/tmp/reference.log
  cmp /tmp/source.log /tmp/reference.log
  cmp /tmp/source.log /tmp/expected
  echo "$format: source/reference setters and filtering match"
done
for args in "getflag flags 3" "setflag flags 3=1"; do
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /tmp/source-cli $args >/tmp/source.log
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /firmware/bin/nvram $args >/tmp/reference.log
  cmp /tmp/source.log /tmp/reference.log
  echo "$args: source/reference command behavior matches"
done

'
