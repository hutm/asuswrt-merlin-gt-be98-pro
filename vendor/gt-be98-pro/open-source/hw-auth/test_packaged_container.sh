#!/usr/bin/env bash
set -euo pipefail
suite=/source/vendor/gt-be98-pro/open-source/hw-auth
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
for test in clm handshake composed wrapper; do
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic -I/source/release/src/router/shared "$suite/test_$test.c" -ldl -o /tmp/test
 qemu-arm -L /reference -E LD_LIBRARY_PATH=/reference/lib:/reference/usr/lib /tmp/test libshared.so > "/audit/hw-$test-reference.log"
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so > "/audit/hw-$test-packaged.log"
 cmp "/audit/hw-$test-reference.log" "/audit/hw-$test-packaged.log"
done
"$cc" -O2 -Wall -Wextra -Werror "$suite/test_tables.c" -ldl -o /tmp/tables
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/tables /reference/usr/lib/libshared.so /firmware/usr/lib/libshared.so
"$cc" -O2 -Wall -Wextra -Werror -rdynamic "$suite/test_wrapper_failure.c" -ldl -o /tmp/failure
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/failure libshared.so > /audit/hw-wrapper-failure.log
printf '%s\n' 'Packaged hardware authentication comparisons passed'
