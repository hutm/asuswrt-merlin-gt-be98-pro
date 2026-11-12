set -euo pipefail
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
fixtures=/source/vendor/gt-be98-pro/open-source/amas-wgn
while read -r fixture log; do
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic "$fixtures/$fixture.c" -ldl -o /tmp/test
 qemu-arm -L /reference -E LD_LIBRARY_PATH=/reference/lib:/reference/usr/lib /tmp/test libshared.so > "/audit/wgn-$log-reference.log"
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so > "/audit/wgn-$log-packaged.log"
 cmp "/audit/wgn-$log-reference.log" "/audit/wgn-$log-packaged.log"
 printf 'PASS %s\n' "$log"
done <<'CASES'
test-basic basic
test-subnet subnet
test-serialize serialize
test-allocation allocation
test-settings settings
test-vlan vlan
test-vlan-parser vlan-parser
test-vlan-allocation vlan-allocation
test-bands bands
test-vlan-composed vlan-composed
test-vlan-units vlan-units
test-vlan-shift vlan-shift
test-vlan-settings vlan-settings
test-guest-ifnames guest-ifnames
test-all-ifnames all-ifnames
test-guest-enabled guest-enabled
test-check-settings check-settings
test-guest-vlans guest-vlans
test-vlan-validation vlan-validation
test-guest-subnet guest-subnet
CASES
