#!/usr/bin/env bash
# Invoked inside an isolated Docker container by test-format.py.
set -euo pipefail
mkdir /tmp/mockbin
cat > /tmp/mockbin/nvram <<'MOCK'
#!/bin/sh
case "$2" in
rc_support) echo "$CAPABILITY";;
usb_fatfs_mod|usb_ntfs_mod) echo open;;
apps_state_autofix) echo "$AUTOFIX";;
esac
MOCK
cat > /tmp/mockbin/dosfsck <<'MOCK'
#!/bin/sh
printf '<%s>' "$@" >> /tmp/calls
printf '\n' >> /tmp/calls
if [ "$(wc -l < /tmp/calls)" -gt 1 ]; then exit "$VERIFY_RET"; fi
exit "$CHECK_RET"
MOCK
printf '#!/bin/sh\nexit 0\n' > /tmp/mockbin/free_caches
chmod +x /tmp/mockbin/*
export PATH=/tmp/mockbin:$PATH
export CAPABILITY=open_fs_format AUTOFIX=0 CHECK_RET=0 VERIFY_RET=0
run_check() {
  rm -rf /tmp/fsck_ret
  rm -f /tmp/calls
  bash /source/app_fsck.sh "$1" /dev/test-disk
  test -f "/tmp/fsck_ret/test-disk.$2"
  test ! -f /tmp/fsck_ret/test-disk.2
}
for fs in tfat vfat msdos; do
  AUTOFIX=0 CHECK_RET=0 run_check "$fs" 0
  test "$(cat /tmp/calls)" = '<-n></dev/test-disk>'
  AUTOFIX=0 CHECK_RET=1 run_check "$fs" 1
  AUTOFIX=1 CHECK_RET=1 VERIFY_RET=0 run_check "$fs" 0
  test "$(wc -l < /tmp/calls)" = 2
  test "$(head -n1 /tmp/calls)" = '<-a></dev/test-disk>'
  test "$(tail -n1 /tmp/calls)" = '<-n></dev/test-disk>'
  AUTOFIX=1 CHECK_RET=1 VERIFY_RET=1 run_check "$fs" 1
  AUTOFIX=1 CHECK_RET=2 run_check "$fs" 1
  test "$(wc -l < /tmp/calls)" = 1
done
for fs in ntfs tntfs fuseblk; do
  run_check "$fs" 3
  test ! -f /tmp/calls
  grep -q 'not available' /tmp/fsck_ret/test-disk.log
done
CAPABILITY='' run_check vfat 3
test ! -f /tmp/calls
printf 'Filesystem checker: FAT modes, repair verification, errors and unsupported NTFS state passed\n'
