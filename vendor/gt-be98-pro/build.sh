#!/usr/bin/env bash
set -euo pipefail

kit=${ASUS_BUILD_KIT:?Run iterate.py build to supply the local build kit}
source_dir=${ASUS_SOURCE_DIR:?Run iterate.py build to supply the source checkout}
workspace=$(dirname -- "$source_dir")
container="amng-vendor-39442"
log=${ASUS_BUILD_LOG:?Run iterate.py build to supply the iteration log}

main() {
  docker info >/dev/null
  python3 "$source_dir/vendor/gt-be98-pro/provenance.py" verify
  git -C "$source_dir" merge-base --is-ancestor 69838e4a60564e5550e97f11d8a8cf2c6260c4f0 HEAD
  test -x "$kit/buildroot-2021.02.4/output/host/bin/aarch64-buildroot-linux-gnu-gcc"
  test -x "$kit/buildroot-2021.02.4/output-arm/host/bin/arm-buildroot-linux-gnueabi-gcc"

  # OpenSSL 3.5 requires explicit atomic support with this source-built toolchain.
  git -C "$source_dir" apply --reverse --check "$kit/openssl-3.5-static-atomic.patch"


  docker build -t amng-build --build-arg "BUILD_UID=$(id -u)" \
    --build-arg "BUILD_GID=$(id -g)" - < "$kit/Dockerfile"

  # The absolute workspace mount lets Git resolve this linked worktree's metadata.
  docker run --rm --init --name "$container" \
    -v "$workspace:$workspace" \
    -v "$source_dir:/asuswrt" -v "$kit:/work" \
    -e "BUILD_UID=$(id -u)" -e "BUILD_GID=$(id -g)" -e ASUS_PUBLIC_GPL_38984=y -e "ASUS_OPEN_COMPONENTS=${ASUS_OPEN_COMPONENTS:-n}" \
    amng-build bash -c '
      set -e
      /work/scripts/20-stage-toolchain.sh
      su builder -c "/work/scripts/30-build-firmware.sh gt-be98_pro"
    '

  sha256sum "$source_dir"/release/src-rt-5.04behnd.4916/image/GT-BE98_PRO_*.pkgtb
}

main 2>&1 | tee -a "$log"
