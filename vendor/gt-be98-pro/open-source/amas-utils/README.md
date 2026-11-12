# Mesh utility source component

Implements the 39 functions and six exported tables of `amas_utils.o` for the
experimental GT-BE98_PRO profile (`ASUS_OPEN_COMPONENTS=y`). The component covers
interface lookup, band classification, channel settings, mesh backhaul policy,
MLO settings, LED capability flags and uplink descriptions.

The Makefile compiles the source files separately and combines them into one
relocatable object. The shared-library Makefile selects this object only under
the existing experimental model/profile condition. Hardware drivers remain
separate dependencies.

Run from the repository root:

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/firmware/root \
  bash vendor/gt-be98-pro/open-source/amas-utils/test.sh
```

Tests run in non-root Docker with network disabled and read-only source,
firmware and toolchain mounts. Twenty suites compare the combined source library
with a reference library under ARM emulation and native ASan/UBSan. Most tests
mock settings and device providers; the composed backhaul suite also exercises
source-built interface counts, MLO mode, Wi-Fi policy and settings conversion.
No test connects to a router or writes its persistent settings.
The runner also extracts 31 applicable declarations from Merlin’s public header
and compiles the ARM source against them. The component Makefile has produced
the expected 39 function exports and six data exports.

Compatibility includes repeated settings reads, prefix-based interface matching,
space-only list splitting, buffer truncation, absent-band loop behavior, and
32-bit ARM shifts. Backhaul conversion preserves caller-provided bits in some
outputs; callers must initialize their output storage appropriately.

Defensive differences include rejecting null or empty parser input, ignoring
short repeater interface names, a zero channel-type result for unknown bands,
and ignoring an uplink subtype when its type pointer is null. These inputs are
outside the defined comparison domain of the reference implementation.

Validation is limited to the documented fixtures. Pointer aliasing, arbitrary
malformed settings, hardware boot, persistent settings migration, networking and
performance have not been established. Integration and packaged-image validation
must pass before this component can be treated as a firmware replacement.
