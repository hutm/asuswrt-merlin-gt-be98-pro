# Software authentication-code helper

This source supplies `get_auth_code` through the existing `auth_common.h`
interface. It uses the firmware's OpenSSL SHA-256 implementation. Selection is
limited to `ASUS_OPEN_COMPONENTS=y` on GT-BE98_PRO; other profiles retain their
existing build path. This replaces the software code generator, not the
hardware authentication peer or its callers.

The helper repeatedly hashes the previous hexadecimal digest, the original
input and the iteration number, separated by vertical bars. The first numeric
input token selects the iteration count using signed remainder modulo 200 plus
20. At least one iteration runs. The output capacity also truncates each
formatted hash input, so it must remain part of the compatibility interface.
Null or oversized input, empty/non-numeric tokens, zero and ERANGE parsing use
the existing time-seeded random fallback. Existing errno handling, global
`strtok` state and the `srand`/`rand` calls are retained for compatibility.
These are legacy protocol choices, not a recommendation for a new protocol.

The output is 64 lowercase hexadecimal characters plus a terminator. The
candidate requires a nonnull output and capacity 65 through 513. Invalid
capacities return null/EINVAL without modifying output. The original can write
past undersized buffers or overflow its temporary buffer with oversized
capacities; those unsafe cases are not executed for comparison. If OpenSSL
reports failure, the candidate clears the output and returns null/EIO instead
of using an uninitialized digest. Existing callers may not handle that error
gracefully; no availability guarantee is made for cryptographic backend failure.

The maintained suite checks the single export and 2685 reference comparisons
on ARM and native ASan/UBSan: input lengths 0 through 513, signed numeric values,
parsing boundaries, stale ERANGE, output capacities, fixed/random fallback inputs
and failed clocks. Nine invalid-argument cases and 21 injected hash failures
are tested separately on ARM and with native sanitizers. Time and random inputs
are controlled; no router settings or hardware authentication service is used.

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/sw-auth/test.sh
```

The previous image has ten dynamic consumers: rc, httpd, wlc_monitor, wlc_nt,
wlceventd, cfg_reportstatus, cfg_server, cfg_client, mastiff and aaews. Backward
register inspection at 44 direct call sites indicates capacity 65 throughout.
This is not complete control-flow or runtime-lookup coverage. The source rmd
caller also declares a 65-byte output. The image built from `ddf80d57b1` contains the source helper. Its packaged
libshared matches the stripped build output and passes 2685 comparisons, nine
invalid-argument cases and 21 injected hash failures. The compiled helper object
matches its archive member. Userspace diagnostics are unchanged; all 151 kernel
modules pass static checks. Actual service and hardware-peer interoperability
remain unvalidated.
