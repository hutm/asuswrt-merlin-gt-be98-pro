# Source filesystem integration

The experimental GT-BE98_PRO profile packages the source NTFS formatter and FAT
checker, selects source FAT/NTFS/HFS format commands, and exposes those choices
through an explicit web capability. Other profiles retain their existing choices.
Formatting errors report the existing error state; missing disks are handled
without dereferencing a missing device. This is not yet hardware validated.

Run `python3 vendor/gt-be98-pro/open-source/filesystems/test-format.py` from the
repository root. It extracts the actual start_diskformat function, compiles it
with mocked disk operations under ASan/UBSan in Docker, and tests command
selection, literal labels, formatter errors and missing disks. A Node test runs
the actual UI option-selection block with mocked NVRAM/DOM values. These tests
never format a block device and do not validate mounting, hotplug or flash safety.

Separately, the existing ARM source tools formatted disposable 64MiB FAT32 and
NTFS files under QEMU. FAT checking and NTFS metadata inspection succeeded.
This is a smoke test of the existing dosfstools 3.0.12 and ntfs-3g 2022.10.3 code,
not a security audit of those versions. HFS formatter/checker smoke tests are
recorded in the parent document.

Outstanding: package verification in a full image, FAT checker script integration,
and honest handling of open NTFS consistency checks. ntfsfix does not replace a
complete NTFS consistency checker. Filesystem replacement is not complete while
these integration gaps remain. Mounted USB storage and formatting on router
hardware are untested.
