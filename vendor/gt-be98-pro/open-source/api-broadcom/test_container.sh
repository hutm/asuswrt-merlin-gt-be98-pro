#!/usr/bin/env bash
set -euo pipefail
object=${1:?Supply the compiled platform API object}
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
mkdir -p /evidence
"$cc" -shared "$object" -o /evidence/candidate-api.so
for suite in mib prefix policy wireless-caps; do
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic "/tests/test-$suite.c" -ldl -o /tmp/test
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so > "/evidence/$suite-reference.log"
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test /evidence/candidate-api.so > "/evidence/$suite-candidate.log"
 cmp "/evidence/$suite-reference.log" "/evidence/$suite-candidate.log"
 echo "PASS $suite"
done
"$cc" -O2 -Wall -Wextra -Werror -rdynamic /tests/test-gpio.c -ldl -o /tmp/test-gpio
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test-gpio /evidence/candidate-api.so > /evidence/gpio-candidate.log
"$cc" -O2 -Wall -Wextra -Werror -rdynamic /tests/test-policy.c -ldl -o /tmp/test-policy
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test-policy /evidence/candidate-api.so long-prefix > /evidence/policy-long-prefix.log
