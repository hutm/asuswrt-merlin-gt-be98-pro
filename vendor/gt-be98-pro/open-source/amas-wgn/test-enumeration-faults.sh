set -euo pipefail
tests=$(cd -- "$(dirname -- "$0")" && pwd)
source=/source/vendor/gt-be98-pro/open-source/amas-wgn/guest-network.c
cc=/toolchain/bin/arm-buildroot-linux-gnueabi-gcc
flags='-Dmalloc=wgn_test_malloc -Dfree=wgn_test_free -Dstrdup=wgn_test_strdup'
"$cc" -O2 -Wall -Wextra -Werror -fPIC -shared $flags "$source" -o /tmp/candidate.so
"$cc" -O2 -Wall -Wextra -Werror -rdynamic ${tests}/test-enumeration-faults.c -ldl -o /tmp/test
qemu-arm -L /firmware /tmp/test /tmp/candidate.so > ${tests}/enumeration-faults-arm.log
gcc -O2 -Wall -Wextra -Werror -fPIC -shared -fsanitize=address,undefined $flags "$source" -o /tmp/native.so
gcc -O2 -Wall -Wextra -Werror -rdynamic -fsanitize=address,undefined ${tests}/test-enumeration-faults.c -ldl -o /tmp/native
/tmp/native /tmp/native.so > ${tests}/enumeration-faults-native.log
cmp ${tests}/enumeration-faults-arm.log ${tests}/enumeration-faults-native.log
cat ${tests}/enumeration-faults-arm.log
