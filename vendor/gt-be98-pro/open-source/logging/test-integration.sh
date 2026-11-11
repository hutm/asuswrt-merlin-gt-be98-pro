#!/bin/bash
set -euo pipefail
ulimit -c 0
fixture=${LOGGING_FIXTURE:-/source}
output=${LOGGING_OUTPUT:-/output}
mkdir -p /tmp/router/shared /tmp/router/libasuslog/source /tmp/router/nvram /tmp/router/libasuslog/prebuild/{GT-BE98_PRO,OTHER}
cp "$fixture"/integration-Makefile /tmp/router/libasuslog/Makefile
cp "$fixture"/libasuslog.c /tmp/router/libasuslog/source/
cat > /tmp/router/common.mak <<'MAKE'
TOP := /tmp/router
TOP_PLATFORM := /tmp/router
SRCBASE := /tmp/router
KERNEL_HEADER_DIR := /tmp/router
HND_ROUTER := y
CC := /toolchain/bin/arm-buildroot-linux-gnueabi-gcc
MAKE
CC=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
$CC -shared -fPIC "$fixture"/reference_stubs.c -o /tmp/router/nvram/libnvram.so
$CC -shared -fPIC "$fixture"/reference_stubs.c -o /tmp/router/shared/libshared.so
printf 'baseline GT\n' > /tmp/router/libasuslog/prebuild/GT-BE98_PRO/libasuslog.so
printf 'baseline OTHER\n' > /tmp/router/libasuslog/prebuild/OTHER/libasuslog.so
cd /tmp/router/libasuslog
make ASUS_OPEN_COMPONENTS=y BUILD_NAME=GT-BE98_PRO
cp libasuslog.so /tmp/first.so
make ASUS_OPEN_COMPONENTS=n BUILD_NAME=GT-BE98_PRO
cmp libasuslog.so prebuild/GT-BE98_PRO/libasuslog.so
make ASUS_OPEN_COMPONENTS=y BUILD_NAME=GT-BE98_PRO
cmp libasuslog.so /tmp/first.so
make ASUS_OPEN_COMPONENTS=y BUILD_NAME=OTHER
cmp libasuslog.so prebuild/OTHER/libasuslog.so
make ASUS_OPEN_COMPONENTS=y BUILD_NAME=GT-BE98_PRO
cmp libasuslog.so /tmp/first.so
/toolchain/bin/arm-buildroot-linux-gnueabi-readelf -d libasuslog.so > "$output"/integration-dynamic.txt
/toolchain/bin/arm-buildroot-linux-gnueabi-nm -D --defined-only libasuslog.so > "$output"/integration-exports.txt
cp libasuslog.so "$output"/integration-libasuslog.so
printf 'Integration: source/off/other-model/source transitions passed\n'
