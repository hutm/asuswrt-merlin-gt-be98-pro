# Shared settings encryption and policy replacement

This candidate targets the proprietary `spwenc.o` contribution to libshared.
It is not selected in firmware. Replacing the NVRAM CLI/runtime did not replace
this dependency. Object inspection identifies 37 exported code/data symbols;
an extracted-image scan finds matching imports in 19 userspace ELFs. Symbol-name
matches are not proof of provider binding and omit dynamic/static consumers.

## Implemented sizing helpers

`lengths.c` implements `pw_enc_len`, `pw_enc_blen` and `pw_dec_len`, with checked
byte-count forms for boundary testing. Encoded size rounds input length plus eight
bytes to a 16-byte boundary; Base64 sizes round by three/four-byte groups. Sizes
do not include an output string terminator. Arithmetic uses checked integers,
not the reference's floating-point block rounding. NULL and unrepresentable
int-sized results return -1 with EINVAL/EOVERFLOW. Valid terminated strings remain
the caller's responsibility.

Non-root Docker tests compare native sanitizer, ARM/QEMU and original-library
outputs for all input lengths 0..65536: 65537 records match. Additional source
tests exercise NULL, signed-result overflow and SIZE_MAX without allocating huge
buffers. This does not establish equivalence at large floating-point precision
boundaries; source overflow handling intentionally avoids the reference's unsafe
integer conversions. No encryption, settings writes or hardware calls occur.

Run from the repository root:

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test.sh
```

## Remaining scope

AES/MD5 compatibility, password framing, salt/key derivation, encrypted defaults
and migration, process/name access policy, privacy-policy data and helpers, and
command wrappers remain. Existing source cryptographic libraries should supply
standard primitives where their interfaces can preserve the existing format.
The complete object and its data exports must be accounted for before removing
it from libshared; these three helpers do not constitute that replacement.
Reference binaries and implementation inspection artifacts stay outside Git.
