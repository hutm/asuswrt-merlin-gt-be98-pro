#!/usr/bin/env bash
set -euo pipefail
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
for suite in mib prefix policy wireless-caps; do
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic "/tests/test-$suite.c" -ldl -o /tmp/test
 for root in reference packaged; do
  qemu-arm -L "/$root" -E "LD_LIBRARY_PATH=/$root/lib:/$root/usr/lib" /tmp/test libshared.so > "/evidence/$suite-$root.log"
 done
 cmp "/evidence/$suite-reference.log" "/evidence/$suite-packaged.log"
 echo "PASS packaged $suite"
done
"$cc" -O2 -Wall -Wextra -Werror -rdynamic /tests/test-gpio.c -ldl -o /tmp/test
qemu-arm -L /packaged -E LD_LIBRARY_PATH=/packaged/lib:/packaged/usr/lib /tmp/test libshared.so > /evidence/gpio-packaged.log
"$cc" -O2 -Wall -Wextra -Werror -rdynamic /tests/test-policy.c -ldl -o /tmp/test
qemu-arm -L /packaged -E LD_LIBRARY_PATH=/packaged/lib:/packaged/usr/lib /tmp/test libshared.so long-prefix > /evidence/policy-long-prefix-packaged.log
