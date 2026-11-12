# Mesh backhaul settings helper

This source supplies the GT-BE98 Pro `amas_dwb` settings interface: three
functions and three writable parameter tables. It replaces only this helper,
not the mesh manager, wireless driver or radio firmware. Selection requires
`ASUS_OPEN_COMPONENTS=y` and `BUILD_NAME=GT-BE98_PRO`.

`dwb_init_settings` initializes a missing or empty backhaul band using the
configured rule and interface count, then selects the MLO subunit according to
repeater mode. Interface parsing retains the 31-byte snapshot and ASCII-space
separator behavior; tabs are part of an interface token.

`cm_revert_wifi_parameters` restores the saved SSID and closed-network flag
when a nonnegative band and nonempty saved SSID exist. It removes the saved
values and commits. Its result reports whether restoration was attempted;
settings write or commit errors do not change that legacy result.

`cm_revertSmartConnectParameters` restores selected Smart Connect settings
and removes their backups, skipping the dedicated backhaul interface. Its
integer argument is unused in this model's interface. Only the reference's
selected fields are restored; deleting other backup fields is intentional.
It does not commit. Missing settings read as empty strings. Existing read and
write ordering is preserved for the tested paths.

The comparison suite checks all six exported symbols and 7290 scenarios on ARM,
then compares native ASan/UBSan execution. It covers backhaul bands, missing/empty
settings, Smart Connect and EAP modes, current and saved settings, whitespace,
interface-list truncation, and failed writes/unsets/commits. All settings live in
an in-memory test store; the suite never modifies router settings. Parameter
names use bounded buffers; arbitrarily modified oversized table entries are not
claimed compatible with the original interface.

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/amas-dwb/test.sh
```

The image built from `d57b548de2` includes the source helper. Its packaged
libshared matches the stripped build output and passes all 7290 comparison
scenarios with mocked settings providers. Userspace diagnostics are unchanged,
and all 151 kernel modules pass static checks. Physical mesh operation remains
unvalidated. The startup caller is `rc/init.c`; private consumers may also
invoke this public interface.
