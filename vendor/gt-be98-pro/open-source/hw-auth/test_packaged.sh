#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_REFERENCE_ROOT:?Set to extracted reference firmware root}"
: "${ASUS_IMAGE_RUN:?Set to audited image run directory containing root/}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_REFERENCE_ROOT:/reference:ro" -v "$ASUS_IMAGE_RUN/root:/firmware:ro" \
 -v "$ASUS_IMAGE_RUN:/audit" "${ASUS_COMPONENT_IMAGE:-amng-component-test}" \
 bash /source/vendor/gt-be98-pro/open-source/hw-auth/test_packaged_container.sh
