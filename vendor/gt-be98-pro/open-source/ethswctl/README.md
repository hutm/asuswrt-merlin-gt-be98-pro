# Jumbo-frame status formatter

This source replaces the ten exported formatting functions from the GT-BE98 Pro
shared-library `ethswctl.o`. They print jumbo-frame status for nine port bits;
they do not read registers, issue ioctls, change MTUs or implement acceleration.
The profile selects this object only for `ASUS_OPEN_COMPONENTS=y` on GT-BE98_PRO.

The dispatcher masks its selector to nine bits. Selectors 0 through 8 print one
port; selector 9 prints the full mask followed by all ports. Other selectors
print nothing. Port names, order, whitespace and uppercase mask formatting are
preserved. Individual functions return `puts` results; the all-ports case returns
the final port result. Unused selectors return the masked selector. No public
return declaration was found; preserving observed ARM behavior avoids assuming
callers discard it.

The suite compares all ten exports, ARM output and return values over 528912
calls: every nine-bit mask, selectors 0 through 1023, direct port functions and
four full-width boundary values. Native ASan/UBSan execution must match output;
native libc return values are excluded from cross-architecture comparison.

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/ethswctl/test.sh
```

A scan of 801 ELF files in image `20261112T011455Z-4pii0t_j` found the ten exports
in libshared and no dynamic imports. That does not rule out internal calls or
runtime symbol lookup. The exports are retained. The image built from `4515464505` includes the source
formatter: its packaged libshared matches the stripped build output, and all
528912 output/return comparisons pass against the packaged functions. Userspace
diagnostics are unchanged and all 151 kernel modules pass static checks.
Hardware testing remains pending; this replacement says nothing about driver
source coverage.
