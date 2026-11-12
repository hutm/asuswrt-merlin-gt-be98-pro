# Broadcom utility source selection

The experimental GT-BE98_PRO profile builds bcmutils, bcmxtlv and
bcmwifi_channels from the existing platform source selected by SRCBASE.
The source files retain their original Broadcom license notices. These are
userspace utilities, not replacements for the wireless or acceleration drivers.
Other profiles retain their existing object selection.

The comparison suite takes actual source-built objects from the shared build:

```sh
ASUS_REFERENCE_ROOT=/absolute/reference/root \
ASUS_BCM_OBJECT_DIR=/absolute/source/release/src/router/shared \
bash vendor/gt-be98-pro/open-source/bcm-utilities/test.sh
```

It compares externally defined symbol names/types with the pinned vendor objects,
then compares original ARM objects, source-built ARM objects, and native source
execution under AddressSanitizer and UndefinedBehaviorSanitizer. All compilation
and execution occurs in non-root Docker without network access.

Coverage: 135428 utility records (CRC/checksum, bit count, power conversion and
integer square root), 8192 TLV packing/unpacking cases including capacity and
canary checks, and all 65536 channel-specification encodings. These tests do not
exercise every exported function or hardware behavior. Native compilation uses
-Ulinux to avoid a vendor header selecting kernel-only stdarg headers from the
host kernel version; the firmware compilation retains its actual platform flags.

Full-image integration and hardware validation remain pending.
