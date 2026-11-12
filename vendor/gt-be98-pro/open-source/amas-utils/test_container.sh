set -euo pipefail
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
python3 /tests/check_declarations.py /source/release/src/router/shared/shared.h > /tests/public-declarations.h
sources=()
for file in /tests/*.c; do
 case "$file" in /tests/test-*) continue;; esac
 sources+=("$file")
done
"$cc" -O2 -Wall -Wextra -Werror -fPIC -include /tests/public-declarations.h -shared "${sources[@]}" -o /tests/combined-arm.so
gcc -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -Dsyslog=test_syslog -O2 -Wall -Wextra -Werror -fPIC -shared -fsanitize=address,undefined "${sources[@]}" -o /tmp/combined-native.so
mkdir -p /tests/combined
for fixture in /tests/test-*.c; do
 name=${fixture##*/}; name=${name#test-}; name=${name%.c}
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic "$fixture" -ldl -o /tmp/test
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so > "/tests/combined/$name-reference.log"
 qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test /tests/combined-arm.so > "/tests/combined/$name-arm.log"
 cmp "/tests/combined/$name-reference.log" "/tests/combined/$name-arm.log"
 gcc -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -Dsyslog=test_syslog -O2 -Wall -Wextra -Werror -rdynamic -fsanitize=address,undefined "$fixture" -ldl -o /tmp/native
 /tmp/native /tmp/combined-native.so > "/tests/combined/$name-native.log"
 cmp "/tests/combined/$name-reference.log" "/tests/combined/$name-native.log"
 echo "PASS $name"
done
