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
$compiler -std=c11 -Wall -Wextra -Werror "$component/profile.c" "$component/test_profile_export.c" -o /tmp/export-profile
qemu-arm -L /toolchain/arm-buildroot-linux-gnueabi/sysroot /tmp/export-profile /tmp/source-HDR2
for format in HDR1 HDR2 source-HDR2; do
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /tmp/source-cli restore /tmp/$format >/tmp/source.log
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /firmware/bin/nvram restore /tmp/$format >/tmp/reference.log
  cmp /tmp/source.log /tmp/reference.log
  cmp /tmp/source.log /tmp/expected
  echo "$format: source/reference setters and filtering match"
done
for args in "getflag flags 3" "setflag flags 3=1" "restart" "commit" "commit restart" "get key" "set key=value" "set key=a=b" "set key=" "set key" "unset key"; do
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /tmp/source-cli $args >/tmp/source.log
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /firmware/bin/nvram $args >/tmp/reference.log
  cmp /tmp/source.log /tmp/reference.log
  echo "$args: source/reference command behavior matches"
done

qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so /tmp/source-cli savefile /tmp/export >/tmp/source.log
mv /tmp/export /tmp/source-export
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so /firmware/bin/nvram savefile /tmp/export >/tmp/reference.log
cmp /tmp/source.log /tmp/reference.log
cmp /tmp/source-export /tmp/export
printf "key=one\nempty=\nequals=a=b\n" >/tmp/expected-export
cmp /tmp/export /tmp/expected-export
echo "savefile: source/reference exports and status match"

for command in show dump getall; do
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so -E DUMP_FIXTURE=1 /tmp/source-cli $command >/tmp/source.log 2>/tmp/source.err
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so -E DUMP_FIXTURE=1 /firmware/bin/nvram $command >/tmp/reference.log 2>/tmp/reference.err
  cmp /tmp/source.log /tmp/reference.log
  cmp /tmp/source.err /tmp/reference.err
  printf "key=one\nXX ILLEGAL nvram\nempty=\nequals=a=b\n" >/tmp/expected-dump
  cmp /tmp/source.log /tmp/expected-dump
  echo "$command: filtering, empty slots and size report match"
done

qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so -E SAVE_FIXTURE=1 /tmp/source-cli save /tmp/source-backup
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so -E SAVE_FIXTURE=1 /firmware/bin/nvram save /tmp/reference-backup
python3 - <<"PYTHON"
from pathlib import Path
def records(path):
    data=Path(path).read_bytes()
    assert data[:4]==b"HDR2"
    size=int.from_bytes(data[4:7],"little")
    assert len(data)==size+8
    payload=bytes(0 if c>=253 else (255+data[7]-c)&255 for c in data[8:])
    return payload.rstrip(b"\0").split(b"\0")
expected=[b"lan_ipaddr=192.0.2.1",b"test_exact=decrypted",b"empty=",b"equals=a=b"]
assert records("/tmp/source-backup")==records("/tmp/reference-backup")==expected
print("save: encrypted-table selection and decoded backup records match")
PYTHON

if qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so -E SAVE_FIXTURE=1 /tmp/source-cli save /dev/full; then
  echo "save incorrectly succeeded on a full output device" >&2
  exit 1
fi
echo "save: output failure returns nonzero"

for fault in readfail unterminated oversized decryptfail decryptunterminated expansion; do
  for command in save fb_save; do
    printf "existing backup" >/tmp/preserved
    if qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
      -E LD_PRELOAD=/tmp/stubs.so -E SAVE_FIXTURE=1 -E SAVE_FAULT=$fault \
      /tmp/source-cli $command /tmp/preserved; then
      echo "$command unexpectedly accepted $fault" >&2
      exit 1
    fi
    test "$(cat /tmp/preserved)" = "existing backup"
  done
  echo "backup input failure $fault: rejected before destination changes"
done

for executable in /tmp/source-cli /firmware/bin/nvram; do
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so -E ERASE_FIXTURE=1 $executable erase >/tmp/erase.log
  printf "SYSTEM[hnd-erase nvram]\n" >/tmp/expected-erase
  cmp /tmp/erase.log /tmp/expected-erase
done
if qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so -E ERASE_FIXTURE=1 -E ERASE_FAILURE=1 /tmp/source-cli erase; then
  echo "erase ignored helper failure" >&2
  exit 1
fi
echo "erase: reference helper route matches and source reports helper failure (all operations intercepted)"

for executable in /tmp/source-cli /firmware/bin/nvram; do
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so -E ERASE_FIXTURE=1 $executable godefault >/tmp/default.log
  printf "UNLINK[/data/.kernel_nvram.setting]\nSYNC\n" >/tmp/expected-default
  cmp /tmp/default.log /tmp/expected-default
done
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so -E ERASE_FIXTURE=1 -E UNLINK_FAULT=missing \
  /tmp/source-cli godefault >/tmp/default.log
cmp /tmp/default.log /tmp/expected-default
if qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so -E ERASE_FIXTURE=1 -E UNLINK_FAULT=denied \
  /tmp/source-cli godefault >/tmp/default.log; then
  echo "godefault ignored deletion failure" >&2
  exit 1
fi
printf "UNLINK[/data/.kernel_nvram.setting]\n" >/tmp/expected-default
cmp /tmp/default.log /tmp/expected-default
echo "godefault: reference sequence matches; absent file and deletion failures handled (intercepted)"

for pair in "source /tmp/source-cli" "reference /firmware/bin/nvram"; do
  set -- $pair
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so -E FEEDBACK_FIXTURE=1 $2 fb_save /tmp/$1-feedback
done
python3 - <<"PYTHON"
from pathlib import Path
def records(path):
    data=Path(path).read_bytes()
    assert data[:4]==b"HDR2" and len(data)==8+int.from_bytes(data[4:7],"little")
    return bytes(0 if c>=253 else (255+data[7]-c)&255 for c in data[8:]).rstrip(b"\0").split(b"\0")
a=records("/tmp/source-feedback"); b=records("/tmp/reference-feedback")
assert a==b,(a,b)
expected=[b"http_username=xxxxxxxx",b"http_passwd=xxxxxxxx",b"custom_password=xxxxxx",b"wl0_wpa_psk=xxxxxx",b"acc_list=xxxxxxxx>xxxxxxxx",b"vpnc_clientlist=<desc>type>1>alice>xxxxxx",b"cloud_sync=<type>user>xxxxxx>xxx>rule>xxx>1",b"wan_pppoe_username=xxxxx@example.test",b"lan_ipaddr=192.0.2.1"]
assert a==expected,a
print("feedback: source/reference records match explicit redaction expectations")
PYTHON

printf "existing backup" >/tmp/preserved
if qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so -E FEEDBACK_EXPANSION=1 /tmp/source-cli fb_save /tmp/preserved; then
  echo "feedback export ignored redaction expansion" >&2
  exit 1
fi
test "$(cat /tmp/preserved)" = "existing backup"
echo "feedback: oversized redaction rejected without replacing backup"

for command in save_ap save_rp_2g save_rp_5g save_rp_5g2 setter getter unsetter showmore restoremore fb_save_more; do
  printf "existing backup" >/tmp/preserved
  if qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /tmp/source-cli $command /tmp/preserved >/tmp/unknown.log; then
    echo "$command unexpectedly accepted" >&2
    exit 1
  fi
  test ! -s /tmp/unknown.log
  test "$(cat /tmp/preserved)" = "existing backup"
done
for command in get set unset save restore fb_save; do
  if qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so /tmp/source-cli $command >/tmp/missing.log; then
    echo "$command accepted missing arguments" >&2
    exit 1
  fi
  test ! -s /tmp/missing.log
done
echo "dispatch: unsupported modes, command prefixes and missing arguments rejected without backend calls"

for option in 0 1; do
  for pair in "source /tmp/source-cli" "reference /firmware/bin/nvram"; do
    set -- $pair
    qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
      -E LD_PRELOAD=/tmp/stubs.so -E SAVE_FIXTURE=1 $2 save /tmp/$1-option-$option $option
  done
done
python3 - <<"PYTHON"
from pathlib import Path
def records(path):
    data=Path(path).read_bytes()
    assert data[:4]==b"HDR2" and len(data)==8+int.from_bytes(data[4:7],"little")
    return bytes(0 if c>=253 else (255+data[7]-c)&255 for c in data[8:]).rstrip(b"\0").split(b"\0")
for option in (0,1):
    expected=[b"lan_ipaddr=192.0.2.1",b"test_exact="+(b"ciphertext" if option else b"decrypted"),b"empty=",b"equals=a=b"]
    assert records(f"/tmp/source-option-{option}")==records(f"/tmp/reference-option-{option}")==expected
print("save options: raw and decrypted profiles match the reference records")
PYTHON
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
  -E LD_PRELOAD=/tmp/stubs.so -E SAVE_FIXTURE=1 -E SAVE_FAULT=decryptfail \
  /tmp/source-cli save /tmp/raw-bypass 1 get key >/tmp/raw-chain.log
printf "GET[key]\n" >/tmp/expected-chain
cmp /tmp/raw-chain.log /tmp/expected-chain
echo "raw save: bypasses decryption and preserves following commands"

for pair in "source /tmp/source-cli" "reference /firmware/bin/nvram"; do
  set -- $pair
  qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib \
    -E LD_PRELOAD=/tmp/stubs.so -E FEEDBACK_MATRIX=1 $2 fb_save /tmp/$1-matrix
done
python3 - <<"PYTHON"
from pathlib import Path
def records(path):
    data=Path(path).read_bytes()
    assert data[:4]==b"HDR2" and len(data)==8+int.from_bytes(data[4:7],"little")
    return dict(item.split(b"=",1) for item in bytes(0 if c>=253 else (255+data[7]-c)&255 for c in data[8:]).rstrip(b"\0").split(b"\0"))
a=records("/tmp/source-matrix");b=records("/tmp/reference-matrix")
differences=[(k.decode(),a[k].decode(),b[k].decode()) for k in a if a[k]!=b[k]]
print("wireless feedback differences:",differences)
assert a==b
print("wireless feedback: 735 credential-pattern records match")
PYTHON

'
