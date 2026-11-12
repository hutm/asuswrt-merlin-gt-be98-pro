# Hardware authentication helpers

This source supplies the authentication helper interfaces and writable tables
used by libshared. It is selected only for GT-BE98_PRO with
`ASUS_OPEN_COMPONENTS=y`. It replaces a userspace object, not a wireless driver
or radio firmware. Wireless version queries still use the existing `wl` tool.

The implementation preserves product and application selection, GPIO matching,
wireless version checks, handshake formatting and the public validation wrapper.
The four public tables retain their ARM layout. The software digest helper is
provided separately by the OpenSSL-based sw-auth component.

Run the maintained comparison suite against an extracted reference image:

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/hw-auth/test.sh
```

The suite checks exported symbol names and data sizes, 4653 wireless query
cases, 792 handshake cases, 640 composed authentication cases, 486 wrapper
cases and 48 public table fields. ARM emulation and native ASan/UBSan runs
compare outputs and provider traces. Five wrapper failure cases and 14 interface
validation cases exercise candidate error handling. Providers for settings,
clock, randomness and wireless commands are controlled; no router is accessed.

Intentional differences: invalid interface names are rejected before shell
execution; handshake arguments and output capacity are validated; hash failures
propagate; the wrapper rejects null arguments and failed helpers. Early handshake
failure uses an initialized empty key rather than unspecified stack contents.
Application identifiers are copied with a bound. Optional debug file and system
log output is omitted. These changes are not claims of identical behavior for
invalid inputs or arbitrarily modified public tables.

Legacy constraints remain: static buffers are not reentrant; wireless queries
share `/tmp/hw_auth_clm` and retain existing command/read behavior; the random
retry loop can wait for time to advance on a collision; tunnel applications wait
for wireless readiness without a timeout. Those choices preserve existing
protocol behavior and are not recommended for new authentication protocols.

The firmware built from `cdf046bb2b` packages this source implementation. Its
libshared matches the stripped build output, and the compiled helper matches its
archive member. The packaged library passes all four comparison suites, the
48-field table comparison and five wrapper failure cases. Userspace diagnostics
are unchanged, and all 151 kernel modules pass static checks. Hardware boot and
real service interoperability remain unvalidated.

To repeat packaged validation, set `ASUS_REFERENCE_ROOT` to the preceding image's
extracted root, set `ASUS_IMAGE_RUN` to the new audit directory containing `root/`,
and run `bash vendor/gt-be98-pro/open-source/hw-auth/test_packaged.sh`. Comparison
logs are saved in the audit directory.
