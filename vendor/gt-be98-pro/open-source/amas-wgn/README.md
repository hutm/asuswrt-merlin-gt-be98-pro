# Guest-network shared helpers

This source implements the 32 exports of `amas_wgn_shared.o` for the experimental
GT-BE98_PRO profile (`ASUS_OPEN_COMPONENTS=y`). It covers subnet and VLAN records,
settings access, wireless unit mapping, interface enumeration, conflict handling,
and VLAN/subnet lookup. Wireless drivers and hardware acceleration remain separate
dependencies.

The build integration selects one source object behind the existing model/profile
condition. Other profiles retain their existing implementation. `subnet.h` and
`vlan.h` contain the public record layouts without the router headers' unrelated
dependencies. The test runner extracts the actual declarations from Merlin's
`amas_wgn_shared.h` and checks every field's offset and size on ARM and the native
host. Upstream declaration changes therefore fail validation rather than silently
changing the ABI.

Run the maintained suite from the repository root:

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
  bash vendor/gt-be98-pro/open-source/amas-wgn/test.sh
```

The runner uses a disposable, non-root Docker container with no network and
read-only source, firmware and toolchain mounts. Twenty comparison suites exercise
the combined source library on ARM and under native ASan/UBSan. Settings, diagnostic
logging and the kernel VLAN configuration file are mocked; tests do not access a
router. Tests compare output bytes, return values and relevant provider calls.

Compatibility details retained include prefix-based lookups, 32-byte intermediate
serialization buffers, decimal wireless-mask parsing alongside hexadecimal mask
assignment, and existing interface-list truncation behavior. The all-interface
function expects an already terminated output string and appends to it. The
per-band function clears the output. These interfaces are not interchangeable.

Intentional defensive differences:

- Settings setters reject incoming or combined lists above eight records.
- A failed read of retained records aborts an update without writing settings.
- Default-setting checks skip updates when allocation fails.
- Band-counter indices are bounded, and VLAN-file scan fields are width-limited.
- VLAN validation and subnet lookup reject null input pointers.

The VLAN-validation API lacks an output capacity argument; callers still must
provide enough space for the parent interface name. Other pointer and string
validity requirements remain part of the existing API. Source comparison tests
are evidence of compatibility for their covered cases, not proof of hardware
behavior. Firmware integration, packaged checks and hardware validation must be
completed before this replacement is considered suitable for installation.

The experimental image built from `50465117c5` packages this implementation.
All 20 suites pass against its packaged library, which matches the stripped
build output. The guest-network object matches the source-built archive member;
`libshared` now contains four remaining vendor-matching objects. Userspace
diagnostics are unchanged, and all 151 kernel modules pass static checks.
Twelve additional allocation-failure tests cover enumeration and subnet lookup
buffer handling and cleanup. Hardware validation remains pending.

For packaged validation, set `ASUS_REFERENCE_ROOT` to the preceding extracted
root and `ASUS_IMAGE_RUN` to the new audit directory containing `root/`, then run
`bash vendor/gt-be98-pro/open-source/amas-wgn/test_packaged.sh`.
