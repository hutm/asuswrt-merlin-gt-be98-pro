# ASUS platform API source

Experimental GT-BE98 Pro replacement for the prebuilt `api-broadcom.o`. The
source comes from ASUS GPL package 3.0.0.6.102.38984. `provenance.json` records
the official download URL, archive checksum, and original member checksum.
`asus-original.c` preserves the original source; `api-broadcom.c` applies only
the changes recorded in `source-fixes.patch`. `LICENSE.asus` preserves the
archive's top-level GPL v2 license text; the source has no individual header.

The patch fixes initializer braces, pointer types, a forward declaration,
GPIO read termination and a missing return, and a Smart Connect key buffer
that was too small for the supported interface-prefix buffer. Hardware
providers and acceleration drivers remain external dependencies.

Build selection is limited to `ASUS_OPEN_COMPONENTS=y` and `GT-BE98_PRO`.
The upstream object-selection path remains in place for other profiles.

## Validation

`test_container.sh` accepts the compiled object and runs in the non-root
`amng-component-test` container with source mounted at `/tests`, writable
evidence at `/evidence`, the ARM toolchain at `/toolchain`, and a reference
firmware root at `/firmware`. It compares counter reads, Smart Connect prefix
selection and policy writes, and wireless capability parsing. GPIO cases
check the candidate's defined behavior; they do not compare undefined vendor
behavior. Providers are mocked. No hardware, boot, or performance validation
is implied. The remaining exported functions require additional validation.

When updating ASUS source, preserve the new original and provenance, reapply
only necessary fixes, rerun comparisons, and rebuild and audit the full image.
