#!/usr/bin/env bash
set -euo pipefail
suite=/source/vendor/gt-be98-pro/open-source/ethctl-command
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
"$cc" -O2 -Wall -Wextra -Werror -fPIC -c "$suite/commands.c" -o /tmp/ethctl_cmd.o
for variant in reference candidate; do
 object=/source/release/src/router/shared/prebuild/GT-BE98_PRO/ethctl_cmd.o
 [ "$variant" != candidate ] || object=/tmp/ethctl_cmd.o
 /toolchain/bin/arm-buildroot-linux-gnueabi-nm --extern-only --defined-only "$object" | awk '{print $2,$3}' | sort > "/tmp/$variant.exports"
done
diff -u /tmp/reference.exports /tmp/candidate.exports
"$cc" -shared /tmp/ethctl_cmd.o -o /tmp/candidate.so
gcc -O2 -Wall -Wextra -Werror -fPIC -shared -fsanitize=address,undefined "$suite/commands.c" -o /tmp/native.so
for test in simple mii wirespeed reset media; do
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic "$suite/test-$test.c" -ldl -o /tmp/test
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so 0 > "/tmp/$test-reference.log" 2> "/tmp/$test-reference.err"
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test /tmp/candidate.so 0 > "/tmp/$test-candidate.log" 2> "/tmp/$test-candidate.err"
 cmp "/tmp/$test-reference.log" "/tmp/$test-candidate.log"
 cmp "/tmp/$test-reference.err" "/tmp/$test-candidate.err"
 gcc -O2 -Wall -Wextra -Werror -rdynamic -fsanitize=address,undefined "$suite/test-$test.c" -ldl -o /tmp/native
 /tmp/native /tmp/native.so 0 > "/tmp/$test-native.log" 2> "/tmp/$test-native.err"
 cmp "/tmp/$test-reference.log" "/tmp/$test-native.log"
 cmp "/tmp/$test-reference.err" "/tmp/$test-native.err"
 tail -1 "/tmp/$test-reference.err"
done
"$cc" -O2 -Wall -Wextra -Werror "$suite/test-lookup.c" -ldl -o /tmp/lookup
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/lookup libshared.so /tmp/candidate.so
gcc -O2 -Wall -Wextra -Werror -rdynamic -fsanitize=address,undefined "$suite/test-no-common.c" -ldl -o /tmp/guard
/tmp/guard /tmp/native.so > /tmp/guard.log 2> /tmp/guard.err
printf '%s\n' 'Bounded no-common-media case passed'
