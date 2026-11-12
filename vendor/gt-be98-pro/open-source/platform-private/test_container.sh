#!/usr/bin/env bash
# Inside non-root amng-component-test: /suite ro, /evidence rw, /toolchain ro, /firmware ro.
set -euo pipefail
: "${CANDIDATE_LIB:?Set to candidate library path}"
REFERENCE_LIB=${REFERENCE_LIB:-libshared.so}
cc=/toolchain/bin/arm-linux-gcc
mkdir -p /evidence
for name in sysfs link-status phy-status wan-status rtk-properties lan-status phy-composed rtk-counters switch-dispatch switch-registers mdio-read phy-operation phy-bits vendor-ie vendor-ie-remove channel-settings channel-select channel-list board-id port-status image-diagnostics image-crc trx-check trx-files package-board; do
 "$cc" -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -O2 -fno-builtin -Wall -Wextra -Werror -rdynamic /suite/test-$name.c -ldl -o /tmp/test
 for variant in reference candidate; do
  library=$REFERENCE_LIB; args=()
  if [ "$variant" = candidate ]; then
   library=$CANDIDATE_LIB
   case $name in phy-operation|vendor-ie) args=(candidate);; esac
  fi
  if [ "$name" = package-board ]; then
   qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test "$library" "${args[@]}" > /evidence/$name-$variant.log 2>&1
  else
   qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test "$library" "${args[@]}" > /evidence/$name-$variant.log 2> /evidence/$name-$variant.stderr
  fi
 done
 cmp /evidence/$name-reference.log /evidence/$name-candidate.log
 echo "PASS $name"
done
for name in vendor-ie vendor-ie-remove channel-settings channel-select channel-list image-diagnostics image-crc trx-check package-board; do
 "$cc" -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -O2 -fno-builtin -Wall -Wextra -Werror -rdynamic /suite/test-$name.c -ldl -o /tmp/test
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test "$CANDIDATE_LIB" invalid > /evidence/$name-boundaries.log 2> /evidence/$name-boundaries.stderr
 echo "PASS $name boundaries"
done
"$cc" -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -O2 -fno-builtin -Wall -Wextra -Werror -rdynamic /suite/test-image-crc-errors.c -ldl -o /tmp/test
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test "$CANDIDATE_LIB" > /evidence/image-crc-errors.log
 echo "PASS image-crc-errors"
