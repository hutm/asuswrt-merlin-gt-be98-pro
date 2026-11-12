# Ethernet command helper

This source supplies `command_lookup` and its seven command callbacks for
GT-BE98_PRO with `ASUS_OPEN_COMPONENTS=y`. It replaces the userspace object in
libshared. The Ethernet driver, accelerator and standalone vendor ethctl utility
remain separate dependencies.

The table preserves names, argument counts and help text for media selection,
PHY reset, register access, power control, virtual-port control, statistics and
wirespeed control. Callbacks retain the existing request layouts, settings,
status messages and return values, including legacy quirks: wirespeed returns
one even on successful operations, and vport enable/disable perform no ioctl.
The source keeps ARM 32-bit numeric parsing semantics on native test hosts.

The maintained Docker suite compares 320,794 callback cases on ARM and native
ASan/UBSan. It exercises request fields and call order, driver mutations of
shared request storage, failure injection, media and PHY capabilities, register
values, argument combinations and numeric boundaries. Another 7,730 lookup
cases compare the public table and unknown-command handling. The object exports
only `command_lookup`. Device calls and sleep are mocked; no hardware is touched.

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/ethctl-command/test.sh
```

Intentional differences: the media-priority search is bounded to its five valid
entries; a dedicated sanitizer test covers no common media. SERDES request
storage is initialized instead of retaining unspecified stack bytes. Only
defined request fields are compared for those reference requests. The vport
query retains a pointer to static storage instead of a returned stack frame.
Null lookup returns null, and missing option values return errors rather than
dereferencing null. Other malformed pointers remain outside the interface.

The private request numbers and layouts are specific to this router's reference
driver ABI. Future SDK changes need renewed comparison and hardware tests.
Static request storage and existing callback behavior do not provide thread
safety. The image built from `0efb344dd0` packages this source implementation.
The packaged library matches its stripped build output and the helper matches
its archive member. All 320,794 callback comparisons, 7,730 lookup checks and
the bounded no-common-media test pass against the packaged library. Userspace
diagnostics are unchanged, and all 151 kernel modules pass static checks.
Hardware validation remains pending.

For packaged validation, set `ASUS_REFERENCE_ROOT` to the preceding extracted
root and `ASUS_IMAGE_RUN` to the new audit directory containing `root/`, then
run `bash vendor/gt-be98-pro/open-source/ethctl-command/test_packaged.sh`.
Comparison logs are saved in the audit directory.
