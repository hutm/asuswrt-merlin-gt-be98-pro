set -euo pipefail
suite=/source/vendor/gt-be98-pro/open-source/ethctl-command
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
for test in simple mii wirespeed reset media; do
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic "$suite/test-$test.c" -ldl -o /tmp/test
 qemu-arm -L /reference -E LD_LIBRARY_PATH=/reference/lib:/reference/usr/lib /tmp/test libshared.so 0 > "/audit/ethctl-$test-reference.log" 2> "/audit/ethctl-$test-reference.err"
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so 0 > "/audit/ethctl-$test-packaged.log" 2> "/audit/ethctl-$test-packaged.err"
 cmp "/audit/ethctl-$test-reference.log" "/audit/ethctl-$test-packaged.log"
 cmp "/audit/ethctl-$test-reference.err" "/audit/ethctl-$test-packaged.err"
 tail -1 "/audit/ethctl-$test-packaged.err"
done
"$cc" -O2 -Wall -Wextra -Werror "$suite/test-lookup.c" -ldl -o /tmp/lookup
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/lookup /reference/usr/lib/libshared.so /firmware/usr/lib/libshared.so
"$cc" -O2 -Wall -Wextra -Werror -rdynamic "$suite/test-no-common.c" -ldl -o /tmp/guard
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/guard libshared.so > /audit/ethctl-guard.log 2> /audit/ethctl-guard.err
