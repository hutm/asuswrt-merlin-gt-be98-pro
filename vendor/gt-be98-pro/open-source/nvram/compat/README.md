# NVRAM compatibility helpers

These eight entry points are source candidates for the remaining NVRAM library.
They are not linked into firmware yet. Run from the repository root:

```sh
bash vendor/gt-be98-pro/open-source/nvram/compat/test.sh
```

The runner compiles shared libraries and test drivers inside Docker. Native
execution uses ASan/UBSan; ARM execution uses QEMU. Their deterministic transcripts
must match each other and the recorded reference transcript hashes. Optionally
set ASUS_REFERENCE_LIBRARY to an absolute path to a local libnvram.so to compare
against that library during the same run. The reference input originally tested
has SHA-256 `6eb214a9e4de1269d5467c8ac5d9f3ed4fdfda535d6546ea222fba44d0e35c46`.
No reference library or generated diagnostic output is included here.

The tests cover 1,836 bit-flag records, 18,677 string-helper records and 140 numeric
conversion records. The trimming tests compare complete input buffers and pointer
offsets, not just returned text. Numeric conversion checks status, 32-bit output
and errno. Bit-flag tests use mocked underlying GET/SET calls; other ancillary
symbols are stubbed only to allow optional reference-library loading.

Compatibility details:

- Flags use 32-bit hexadecimal parsing. Missing-key SET writes zero even when
  enabling a bit, matching the existing behavior. Read/modify/write is not atomic.
- Trimming removes the observed space/quote/LF combinations, preserving CR and
  tabs. Returned pointers may point inside the input buffer.
- Wireless names preserve the existing permissive `wl%d.%d` parsing and return
  the suffix after the first underscore. This is not a strict name validator.
- Numeric conversion saturates to ARM signed 32-bit limits on native hosts too.
  NULL input clears errno and leaves output untouched. Error diagnostics to
  stderr are omitted; callers receive status and errno instead. NULL output
  pointers are rejected for non-NULL input.

These fixtures cover the stated cases, not every input or ABI interaction.
Public-library integration, retained-pointer behavior and hardware validation
remain separate work.
