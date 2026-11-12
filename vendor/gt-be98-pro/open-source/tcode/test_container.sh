set -euo pipefail
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
"$cc" -O2 -Wall -Wextra -Werror -fPIC -shared -I/source/release/src/router/shared /source/vendor/gt-be98-pro/open-source/tcode/settings.c /source/vendor/gt-be98-pro/open-source/tcode/tables.c -o /tmp/candidate.so
"$cc" -O2 -Wall -Wextra -Werror -rdynamic /source/vendor/gt-be98-pro/open-source/tcode/test.c -ldl -o /tmp/test
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so > /tmp/reference.log
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test /tmp/candidate.so > /tmp/candidate.log
cmp /tmp/reference.log /tmp/candidate.log
gcc -O2 -Wall -Wextra -Werror -fPIC -shared -fsanitize=address,undefined -I/source/release/src/router/shared /source/vendor/gt-be98-pro/open-source/tcode/settings.c /source/vendor/gt-be98-pro/open-source/tcode/tables.c -o /tmp/native.so
gcc -O2 -Wall -Wextra -Werror -rdynamic -fsanitize=address,undefined /source/vendor/gt-be98-pro/open-source/tcode/test.c -ldl -o /tmp/native
/tmp/native /tmp/native.so > /tmp/native.log
cmp /tmp/reference.log /tmp/native.log

"$cc" -O2 -Wall -Wextra -Werror /source/vendor/gt-be98-pro/open-source/tcode/test_tables.c -ldl -o /tmp/tables
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/tables libshared.so /tmp/candidate.so

make -f /source/vendor/gt-be98-pro/open-source/tcode/Makefile CC="$cc" CFLAGS="-O2 -Wall -Wextra -Werror" CPPFLAGS="-I/source/release/src/router/shared" BUILD_DIR=/tmp/objects OUTPUT=/tmp/tcode.o
for variant in reference candidate; do
 object=/source/release/src/router/shared/prebuild/GT-BE98_PRO/tcode.o
 [ "$variant" != candidate ] || object=/tmp/tcode.o
 /toolchain/bin/arm-buildroot-linux-gnueabi-nm -S --extern-only --defined-only "$object" | awk '{print $3,$4}' | sort > "/tmp/$variant.exports"
 /toolchain/bin/arm-buildroot-linux-gnueabi-nm -S --extern-only --defined-only "$object" | awk '$3 == "D" || $3 == "B" {print $2,$3,$4}' | sort > "/tmp/$variant.data"
done
diff -u /tmp/reference.exports /tmp/candidate.exports
diff -u /tmp/reference.data /tmp/candidate.data
