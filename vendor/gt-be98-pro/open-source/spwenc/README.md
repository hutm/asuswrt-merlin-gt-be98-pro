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

## Decrypted-frame validation

`nv_pw_unframe` validates the full bounded plaintext frame before copying its
text: block alignment, canonical length, zero padding and the lowercase CRC32
prefix must agree. It handles a payload ending exactly at the block boundary
without requiring an out-of-bounds terminator. Short destinations and invalid
frames leave output unchanged. This parser uses an explicit capacity and has a
stricter contract than the public vendor wrapper.

Native sanitizer and ARM tests cover lengths 1..64, exact block boundaries,
corrupted CRC, nonzero padding, extra/truncated blocks, empty input and missing
arguments. Intercepted-AES reference calls decode all 64 valid frames to the same
text and return 1. CRC-corrupted reference calls return 0 and copy the original
encoded input to output. That fallback must be handled by the eventual wrapper;
it must not be mistaken for successful decryption. The frame helper deliberately
rejects it with EBADMSG instead. No real encryption key or settings access occurs
in these tests. `test_frame.sh` now includes both encoding and decoding checks.

## Explicit-input legacy key derivation

`nv_pw_key` derives the existing 32-byte ASCII-hex AES key from an optional salt,
product identifier and six-byte LAN MAC. It reproduces the reference's formatting,
32-byte salt and 63-byte product truncation, and lowercase MD5 output through
OpenSSL EVP. Inputs are explicit: the helper performs no NVRAM access, sleeps or
hardware calls. Invalid MAC syntax and missing required pointers fail without
changing output. This preserves compatibility with an existing legacy format;
it is not a recommendation for a new key-derivation scheme.

The reference harness intercepts device/settings reads and captures the key at
AES initialization. Native sanitizer and ARM/OpenSSL outputs match 512 reference
cases across 256 synthetic device identifiers, salted and unsalted modes, empty
salts, and strings exceeding both truncation limits. No real device identifiers,
passwords or keys are used. Runtime lookup/retry behavior, denied-salt handling,
missing identifiers and permissive malformed-MAC behavior remain separate work.
The helper deliberately requires canonical MAC syntax rather than silently
accepting partial scans. It is not selected in firmware.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_key.sh
```

## Complete explicit-input encryption candidate

`nv_pw_encrypt` composes the source sizing, framing, key derivation and AES helpers
with OpenSSL Base64 encoding. It takes explicit device inputs and output capacity,
returns an empty string for empty plaintext, and leaves output unchanged on error.
Frame, ciphertext scratch and the local derived key are cleared before freeing or
returning. This is a separate checked API, not the public `pw_enc` ABI or its
runtime settings adapter. It is not selected in firmware.

Native sanitizer and ARM runs match the original wrapper byte-for-byte for 1026
cases: all plaintext lengths 0..512 in salted and unsalted modes, varied printable
contents, synthetic MACs/products and salt/product truncation boundaries. The
reference harness uses real reference AES and complete wrapping, with only device
reads intercepted. Its decoder recovers every original plaintext. Source tests
also cover short destinations, invalid device inputs, NULL arguments and empty
input. These checks establish format compatibility for the tested cases, not
complete behavior under allocation faults or missing runtime settings.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_password.sh
```

Complete checked decryption, public ABI adapters, runtime lookup/retry behavior,
settings migration and remaining policy/helper exports still require work before
removing `spwenc.o`.

## Complete checked decryption candidate

`nv_pw_decrypt` validates canonical Base64, decodes whole AES blocks, derives the
explicit-input key, decrypts through OpenSSL and validates the complete frame
before copying plaintext. It rejects noncanonical pad bits, malformed characters,
partial blocks, bad CRC/padding and short output buffers. Failure leaves the
destination unchanged. Temporary decoded frames, canonical encodings and the
local key are cleared during cleanup. Empty input yields an empty string.

The password suite now tests source decryption for all 1026 ciphertext cases
already matched to the original wrapper, including exact-size destinations and
one-byte-short failures. Native sanitizer and ARM tests also cover malformed
Base64/padding, wrong salt, changed ciphertext and NULL/empty arguments. All pass;
the original decoder still round-trips every valid reference ciphertext. The
checked API returns 0 on success and -1 on failure, unlike the vendor's 1/0
valid/fallback result. A public wrapper still must implement those ABI semantics,
including pass-through of undecodable inputs and runtime device lookup.
This candidate remains outside the firmware build.

## Runtime device-input candidate

`nv_pw_inputs_load` accepts callbacks for salt, product, LAN MAC and delay. It
copies each borrowed string before calling the next getter, limits salt/product/
MAC to 32/63/17 characters, and retries empty product and MAC reads up to three
attempts with 500 microsecond delays between attempts. Missing product stays
empty; missing MAC becomes `00:00:00:00:00:00` for key derivation. This copies
values safely but does not provide an atomic snapshot of changing settings.

The test compares 64 native/ARM/reference cases: immediate, second, third and
never-successful product/MAC reads, salted/unsalted operation, NULL/empty missing
values, and oversized strings in a reused getter buffer. Both the complete call
trace and derived key match the original wrapper. Native tests use address and
undefined-behavior sanitizers. The source adapter also rejects a denied-salt
sentinel with EACCES without dereferencing it or changing the output; this safety
case is not executed against the reference. The key helper still requires a
canonical MAC rather than accepting partially parsed malformed addresses.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_inputs.sh
```

This callback adapter is not yet connected to the public password ABI or selected
in firmware. Remaining work includes wrapper fallback semantics, migration and
policy/helper exports before the original object can be removed.
