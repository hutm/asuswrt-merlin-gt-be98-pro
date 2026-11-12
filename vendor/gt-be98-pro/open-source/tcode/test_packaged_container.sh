set -euo pipefail
suite=/source/vendor/gt-be98-pro/open-source/tcode
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
"$cc" -O2 -Wall -Wextra -Werror -rdynamic "$suite/test.c" -ldl -o /tmp/test
qemu-arm -L /reference -E LD_LIBRARY_PATH=/reference/lib:/reference/usr/lib /tmp/test libshared.so > /audit/tcode-reference.log
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/test libshared.so > /audit/tcode-packaged.log
cmp /audit/tcode-reference.log /audit/tcode-packaged.log
"$cc" -O2 -Wall -Wextra -Werror "$suite/test_tables.c" -ldl -o /tmp/tables
qemu-arm -L /firmware -E LD_LIBRARY_PATH=/firmware/lib:/firmware/usr/lib /tmp/tables /reference/usr/lib/libshared.so /firmware/usr/lib/libshared.so
