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
Full firmware integration and hardware behavior remain to be validated.
