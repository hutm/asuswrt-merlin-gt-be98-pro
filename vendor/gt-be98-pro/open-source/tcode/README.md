# Territory and feature settings

This component supplies the ten function exports and thirteen writable tables
used by the GT-BE98 Pro territory-settings object. It is selected only when
`ASUS_OPEN_COMPONENTS=y`, `BUILD_NAME=GT-BE98_PRO` and territory support is enabled.
The existing region, language, default-value and support tables are preserved;
this change does not request different wireless country settings.

The implementation covers default lookup, control flags, Singapore-mode settings
updates, CN/EU predicates and the existing ASUS DDNS eligibility decision. It
preserves settings-provider call order, commit and notification behavior, the
six-character territory lookup bound, sixteen-byte region buffers and the
64-byte coordinate parsing bound. Signed hexadecimal parsing retains the ARM
32-bit range and ERANGE behavior on native test hosts. Flag shifts reproduce
the ARM register-shift result without invoking undefined C shifts.

Run the comparison suite with an extracted reference firmware root:

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/tcode/test.sh
```

The maintained suite passes 28,110 comparisons on ARM and native ASan/UBSan.
It covers real default-table combinations, synthetic selection and fallback
rows, missing and empty settings, region prefixes, coordinate fields, long
strings, flag shifts, numeric overflow, settings writes and notifications.
All 575 public table fields compare with the reference, and the ARM combined
object matches the reference export names and writable-data sizes/types.
Settings providers are mocked; no router settings are read or changed.

Valid string arguments are required by the original public interfaces; null
function arguments and malformed caller-mutated tables are not covered. Empty
reserved tables preserve their ARM storage, without claiming an interface for
populating them. Testing is limited to this router's tables and reference build.
The image built from `2f2602a4d2` packages the source implementation: its stripped
libshared matches the build output and the compiled helper matches its archive
member. Packaged tests pass all 28,110 behavior comparisons and 575 table fields.
Userspace diagnostics are unchanged, and all 151 kernel modules pass static
checks. Router hardware behavior remains untested.

To repeat packaged checks, set `ASUS_REFERENCE_ROOT` to the preceding extracted
root, set `ASUS_IMAGE_RUN` to the new audit directory containing `root/`, and run
`bash vendor/gt-be98-pro/open-source/tcode/test_packaged.sh`. Logs are saved in
the audit directory.
