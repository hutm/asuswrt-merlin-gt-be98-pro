#!/usr/bin/env bash
# Isolated source compilation; does not stage or flash firmware.
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
output=${1:?Usage: build-ntfs.sh /absolute/empty-output-directory}
mkdir -p "$output"
output=$(realpath "$output")
if [ -n "$(ls -A "$output")" ]; then
    echo 'Output directory must be empty' >&2
    exit 1
fi
printf '%s\n' "$(git rev-parse HEAD)" > "$output/source-commit"
docker image inspect amng-build --format '{{.Id}}' > "$output/container-image"
docker run --rm --user "$(id -u):$(id -g)" \
    -v "$root:/source:ro" -v "$kit:/work:ro" -v "$output:/build" \
    -w /build amng-build bash -c '
set -euo pipefail
export PATH=/work/buildroot-2021.02.4/output-arm/host/bin:$PATH
export CC=arm-buildroot-linux-gnueabi-gcc
export CFLAGS="-Os -Wall -ffunction-sections -fdata-sections"
export LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC -ldl"
/source/release/src/router/ntfs-3g/configure --host=arm-buildroot-linux-gnueabi \
  --enable-shared=no --enable-static=no --disable-library --disable-ldconfig \
  --disable-mount-helper --with-fuse=internal --disable-posix-acls --disable-nfconv \
  --disable-dependency-tracking
make -j4
file src/ntfs-3g
arm-buildroot-linux-gnueabi-readelf -h -d src/ntfs-3g > elf-report.txt
sha256sum src/ntfs-3g > SHA256SUMS
' 2>&1 | tee "$output/build.log"
