set -euo pipefail
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
python3 /tests/check_layout.py /source/release/src/router/shared/amas_wgn_shared.h /tests/
for variant in public candidate; do
 "$cc" -O2 -Wall -Wextra -Werror /tests/$variant-layout.c -o /tmp/layout-arm
 qemu-arm -L /firmware /tmp/layout-arm > /tests/$variant-layout-arm.log
 gcc -O2 -Wall -Wextra -Werror /tests/$variant-layout.c -o /tmp/layout-native
 /tmp/layout-native > /tests/$variant-layout-native.log
 cmp /tests/$variant-layout-arm.log /tests/$variant-layout-native.log
done
cmp /tests/public-layout-arm.log /tests/candidate-layout-arm.log
"$cc" -O2 -Wall -Wextra -Werror -fPIC -shared /tests/guest-network.c -o /tests/component.so
gcc -O2 -Wall -Wextra -Werror -fPIC -shared -fsanitize=address,undefined /tests/guest-network.c -o /tmp/component-native.so
while read -r fixture log; do
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic "/tests/$fixture.c" -ldl -o /tmp/test
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so > "/tests/$log-reference.log"
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test /tests/component.so > "/tests/combined-$log-arm.log"
 cmp "/tests/$log-reference.log" "/tests/combined-$log-arm.log"
 gcc -O2 -Wall -Wextra -Werror -rdynamic -fsanitize=address,undefined "/tests/$fixture.c" -ldl -o /tmp/native
 /tmp/native /tmp/component-native.so > "/tests/combined-$log-native.log"
 cmp "/tests/$log-reference.log" "/tests/combined-$log-native.log"
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
