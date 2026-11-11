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

## AES primitive candidate

`crypto.c` reuses OpenSSL EVP for AES-256 ECB without padding, with checked block
lengths and separate or exactly overlapping input/output buffers. It is not the
vendor context ABI and is not selected in firmware. OpenSSL source already exists
in this repository; the candidate avoids adding another handwritten AES core.
Zero-length operations succeed without touching buffers; invalid arguments and
int-size overflow return errors. A backend failure may leave partial output.

Tests compare native sanitizer/OpenSSL, ARM/OpenSSL and the original library:
the AES-256 key `000102...1f` and plaintext `001122...ff` yield
`8ea2b7ca516745bfeafc49904b496089`. Another 256 deterministic keys/messages each
cover four blocks, exact ciphertext comparisons, decrypt round trips and in-place
encryption. All pass. This establishes the tested primitive behavior only: it
does not validate password framing, key/salt derivation, vendor context layout,
partial-block zero padding, authentication or settings-policy behavior. ECB is
used here to preserve the existing format, not as a new encryption design.

Run with the same extracted reference root:

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_crypto.sh
```

## Password framing candidate

`frame.c` prepares the pre-AES payload using zlib's CRC32: eight lowercase hex
characters, the input text, then zeros up to a 16-byte boundary. Empty input
produces no payload, matching the wrapper's empty result. Capacity is explicit;
invalid arguments, size overflow and short destinations fail before writes.
This is a separate source API, not yet a replacement for `pw_enc` or `pw_dec`.

Native sanitizer and ARM runs match 64 nonempty reference frames across block
boundaries, plus empty-input behavior. Reference tests use synthetic settings,
a synthetic LAN address/salt and intercepted AES functions, so comparisons expose
framing rather than depend on the unfinished key derivation. Independent Python
CRC32 and Base64 decoding confirm the bytes; output-buffer guards and short-buffer
tests pass. The reference wrapper returns zero for these successful cases.
No real passwords, router settings or hardware are accessed.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_frame.sh
```

The CRC is part of format compatibility, not cryptographic authentication.
Decryption validation, key derivation and complete wrapper behavior remain pending.
