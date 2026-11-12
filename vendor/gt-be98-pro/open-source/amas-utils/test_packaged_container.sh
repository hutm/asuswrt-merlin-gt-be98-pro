set -euo pipefail
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
for fixture in /source/vendor/gt-be98-pro/open-source/amas-utils/test-*.c; do
 name=${fixture##*/}; name=${name#test-}; name=${name%.c}
 "$cc" -O2 -Wall -Wextra -Werror -rdynamic "$fixture" -ldl -o /tmp/test
 qemu-arm -L /reference -E LD_LIBRARY_PATH=/reference/lib:/reference/usr/lib /tmp/test libshared.so > "/evidence/$name-reference.log"
 qemu-arm -L /packaged -E LD_LIBRARY_PATH=/packaged/lib:/packaged/usr/lib /tmp/test libshared.so > "/evidence/$name-packaged.log"
 cmp "/evidence/$name-reference.log" "/evidence/$name-packaged.log"
 echo "PASS $name"
done
