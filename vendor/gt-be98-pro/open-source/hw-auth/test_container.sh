#!/usr/bin/env bash
set -euo pipefail
suite=/source/vendor/gt-be98-pro/open-source/hw-auth
router=/source/release/src/router
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
sources=(eligibility component check clm handshake tables validate)
mkdir -p /tmp/auth-arm /tmp/auth-native
for name in "${sources[@]}"; do
 "$cc" -O2 -Wall -Wextra -Werror -fPIC -I"$router/shared" -c "$suite/$name.c" -o "/tmp/auth-arm/$name.o"
 gcc -O2 -Wall -Wextra -Werror -fPIC -fsanitize=address,undefined -I"$router/shared" -c "$suite/$name.c" -o "/tmp/auth-native/$name.o"
done
"$cc" -r /tmp/auth-arm/*.o -o /tmp/hw_auth.o
for variant in reference candidate; do
 object="$router/sw-hw-auth/prebuild/GT-BE98_PRO/hw_auth.o"
 [ "$variant" != candidate ] || object=/tmp/hw_auth.o
 /toolchain/bin/arm-buildroot-linux-gnueabi-nm -S --defined-only --extern-only "$object" | awk '{print $3,$4}' | sort > "/tmp/$variant.exports"
 /toolchain/bin/arm-buildroot-linux-gnueabi-nm -S --defined-only --extern-only "$object" | awk '$3 == "D" {print $2,$3,$4}' | sort > "/tmp/$variant.data"
done
diff -u /tmp/reference.exports /tmp/candidate.exports
diff -u /tmp/reference.data /tmp/candidate.data
software=/source/vendor/gt-be98-pro/open-source/sw-auth/auth_code.c
"$cc" -O2 -Wall -Wextra -Werror -fPIC -I"$router/sw-hw-auth" -I"$router/openssl-3.5/include" -c "$software" -o /tmp/software.o
"$cc" -shared /tmp/hw_auth.o /tmp/software.o -L"$router/openssl-3.5" -lcrypto -o /tmp/libcandidate.so
gcc -O2 -Wall -Wextra -Werror -fPIC -shared -fsanitize=address,undefined /tmp/auth-native/*.o "$software" -I"$router/sw-hw-auth" -lcrypto -o /tmp/libcandidate-native.so
for test in clm handshake composed wrapper; do
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic -I"$router/shared" "$suite/test_$test.c" -ldl -o /tmp/test
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so > "/tmp/$test-reference.log"
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test /tmp/libcandidate.so > "/tmp/$test-candidate.log"
 cmp "/tmp/$test-reference.log" "/tmp/$test-candidate.log"
 gcc -O2 -Wall -Wextra -Werror -rdynamic -fsanitize=address,undefined -I"$router/shared" "$suite/test_$test.c" -ldl -o /tmp/native
 /tmp/native /tmp/libcandidate-native.so > "/tmp/$test-native.log"
 cmp "/tmp/$test-reference.log" "/tmp/$test-native.log"
done
"$cc" -O2 -Wall -Wextra -Werror "$suite/test_tables.c" -ldl -o /tmp/tables
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/tables libshared.so /tmp/libcandidate.so
"$cc" -O2 -Wall -Wextra -Werror -rdynamic "$suite/test_wrapper_failure.c" -ldl -o /tmp/failure
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/failure /tmp/libcandidate.so > /tmp/failure.log
gcc -O2 -Wall -Wextra -Werror -rdynamic -fsanitize=address,undefined "$suite/test_wrapper_failure.c" -ldl -o /tmp/failure-native
/tmp/failure-native /tmp/libcandidate-native.so > /tmp/failure-native.log
cmp /tmp/failure.log /tmp/failure-native.log
"$cc" -O2 -Wall -Wextra -Werror -I"$router/shared" "$suite/test_interface.c" "$suite/clm.c" -o /tmp/interface
qemu-arm -L /firmware /tmp/interface
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -I"$router/shared" "$suite/test_interface.c" "$suite/clm.c" -o /tmp/interface-native
/tmp/interface-native
printf '%s\n' 'Hardware authentication suite passed'
