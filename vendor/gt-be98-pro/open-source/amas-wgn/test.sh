#!/usr/bin/env bash
set -euo pipefail
root=$(git rev-parse --show-toplevel)
kit=${ASUS_BUILD_KIT:-$(dirname "$root")/asuswrt-build}
: "${ASUS_REFERENCE_ROOT:?Set to extracted reference firmware root}"
docker run --rm --network none --user "$(id -u):$(id -g)" \
 -v "$root:/source:ro" -v "$kit/buildroot-2021.02.4/output-arm/host:/toolchain:ro" \
 -v "$ASUS_REFERENCE_ROOT:/firmware:ro" "${ASUS_COMPONENT_IMAGE:-amng-component-test}" \
 bash -euc 'mkdir /tmp/wgn-tests; cp /source/vendor/gt-be98-pro/open-source/amas-wgn/* /tmp/wgn-tests/; sed "s@/tests/@/tmp/wgn-tests/@g" /tmp/wgn-tests/test_container.sh > /tmp/run-wgn.sh; bash /tmp/run-wgn.sh'
