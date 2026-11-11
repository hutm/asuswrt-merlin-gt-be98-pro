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

## Public password wrapper candidates

`wrapper.c` implements `pw_enc` and `pw_dec` using the runtime input adapter and
OpenSSL-backed checked password functions. Empty inputs avoid device reads.
Encryption preserves the legacy caller-allocated `pw_enc_blen(input)+1` buffer
contract. Decryption returns 1 for recovered plaintext and 0 for empty input or
pass-through of an invalid frame, truncating pass-through to `len-1` characters.
Runtime/input/backend errors return -1 and preserve the output. Device-input
copies are cleared after use.

The wrapper suite compares 258 native/ARM/reference cases (lengths 0..128, both
salt modes), complete ciphertexts and decoded values. Wrong-salt fallback and
truncated fallback are checked against the original, with reference allocations
kept large enough for its decoded temporary buffer. Source-only tests cover the
denied salt sentinel, invalid arguments, plaintext pass-through and insufficient
destination space. Native runs use address/undefined-behavior sanitizers.

Intentional safety differences remain: the source decoder enforces `len` for
successful plaintext output, rejects noncanonical Base64/frame encodings before
falling back, and rejects malformed device MACs. It does not reproduce unsafe
reference buffer handling. Allocation-failure behavior is not reference-matched.
These functions are still candidates, not linked into firmware; other exports
from the original object must be supplied before removing it.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_wrapper.sh
```

## Digest and list helper candidates

`helpers.c` supplies `str_to_md5` through OpenSSL EVP and implements
`is_same_pass` / `get_encrypt_list_value` from copied NVRAM strings. Records are
separated by `<`; the first two `>`-separated fields are used, including empty
fields, and later fields are ignored. Lookup returns the first matching user and
leaves the destination unchanged when absent. Comparison continues across
records until both username and password match. For `pptpd_clientlist` and
`vpn_serverx_clientlist`, comparison hashes the stored password first. This MD5
usage preserves the existing format; it is not a new password-storage design.

The helper suite matches native sanitizer, ARM and original-library results for
513 binary-input digests (lengths 0..512), 1280 list lookups and 800 comparisons.
Fixtures include NULL/empty settings, malformed records, empty fields, duplicates,
extra fields, missing users and output capacities 0..7. Source-only checks reject
invalid digest arguments and negative lengths without changing the output.
Allocation/backend failures are not injected or reference-matched. Temporary
list copies are cleared before release. These helpers remain outside firmware
until the complete object replacement is ready.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_helpers.sh
```

## Settings encryption candidate

`settings.c` implements `enc_nvram` plus a capacity-aware `nv_enc_setting` API.
Ordinary values use salted password encryption. The three account-list settings
(`acc_list`, `pptpd_clientlist`, `vpn_serverx_clientlist`) retain usernames and
encrypt the second field, skipping records without a second field and ignoring
later fields. Each resulting record is prepended, preserving the original's
reversal of record order. Empty input returns 0; nonempty processed input returns
1, including lists with no usable records. Errors return -1 without modifying the
output. The legacy entry point requires a 4096-byte destination.

The suite matches original-library output and return values on native sanitizer
and ARM runs for 45 setting/list fixtures and 512 ordinary-value lengths. It
covers malformed/empty fields, extra fields, multiple records and ordinary values
that contain list delimiters. Source-only checks cover short destinations, denied
salt, oversized ciphertext and oversized assembled lists. The source deliberately
rejects over-capacity results rather than silently truncating account data or
allowing an oversized encryption write. Temporary plaintext copies and result
buffers are cleared on cleanup. Allocation failures are not injected. Settings
decryption, conditional updates and migration still need replacements; this
candidate remains outside firmware.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_settings.sh
```

## Settings decryption candidate

The settings transformer now also supplies `dec_nvram` and capacity-aware
`nv_dec_setting`. Ordinary settings use salted password decoding with its
pass-through fallback. Account lists decode each second field and prepend each
usable record, retaining the original record-order behavior. The return value is
that of the last usable record's decoder, not an aggregate success indication;
empty lists or lists without usable records return 0. The checked transformer
keeps output unchanged on errors and rejects an assembled list that exceeds its
4096-byte internal bound or the caller's capacity.

The settings suite adds 604 native/ARM/reference decryption checks: 90 encrypted
fixture decodes with correct/wrong salt, 512 ordinary-value round trips, and two
mixed empty/encrypted list orders that verify the last-record return semantics.
Ciphertext, list output and return values agree across implementations. Native
sanitizer checks also exercise denied salt and short destination errors. Existing
strict frame-validation differences from the password wrapper still apply.
Conditional setting updates and whole-settings migration remain incomplete; this
candidate has not been selected in firmware.

## Conditional settings update candidate

`set_enc_nvram` and capacity-aware `nv_set_enc_setting` now share the settings
transformer. For account lists, a missing or empty submitted password looks up
the first stored record for that user. Its value is retained only when nonempty;
no later duplicate is searched when the first value is empty. Missing users and
empty stored values are omitted. A nonempty submitted password is encrypted.
Ordinary settings behave like `enc_nvram`; no settings are written by this API.

The settings suite adds 300 native/ARM/reference cases combining five setting
names, six stored lists and ten submitted lists. These cover missing and empty
settings, duplicates, omitted/empty/replaced passwords, extra fields and unknown
users. Source tests show that retaining a stored value requires no salt read,
while encrypting a replacement with denied salt fails without changing output.
Short destinations also preserve output. The legacy void lookup helper cannot
distinguish missing values from allocation failure; injected allocation failures
and concurrent settings changes remain untested. All candidates are still outside
the firmware build pending migration and the other remaining object interfaces.

## Migration entry-point candidates

`migration.c` supplies `start_enc_nvram` and `start_dec_nvram`. It walks the
platform defaults table, processes only entries whose `enc` field equals 1,
writes their converted values in table order, then sets `nvram_encrypt` to 1 or
0 respectively. It does not commit settings. The standalone tuple declaration
matches the platform `bcmnvram.h`; ARM size and encryption-field offset are
compile-time checked. Production integration must use a single compatible tuple
declaration with the platform headers.

Unlike the reference, the source stages all conversions before any writes and
returns -1 on conversion/allocation/write failure. A conversion failure therefore
causes no writes. A write failure stops subsequent writes, but previously written
settings cannot be rolled back through this interface; migration is not atomic.
The final mode flag is attempted only after every value write succeeds. Temporary
converted values are cleared when freed. Concurrent setting changes and commit/
power-loss behavior have not been validated.

The migration suite intercepts settings reads, writes and value converters; it
compares 16 native/ARM/reference write traces across table selections, missing
values and both directions. It separately injects a conversion failure and each
of the three write failures in both source directions. Native sanitizer checks
pass. These tests verify orchestration, not end-to-end encryption or a live NVRAM
backend. Startup initialization and the remaining object interfaces still need
work before firmware integration.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_migration.sh
```

## Startup initialization candidate

`init.c` supplies `init_enc_nvram`. It first attempts to decode non-encrypted
default entries with values of length 24..4095, writing only successfully decoded
values. It loads the default `nvram_encrypt_ver`, sets that version when present,
and runs decryption migration. For versions above 2 with a missing salt, OpenSSL
`RAND_bytes` supplies 16 bytes encoded as 32 lowercase hexadecimal characters.
It then runs encryption migration, restores the default `nvram_encrypt` value
when present, and commits. An existing empty salt is considered present, matching
the reference's pointer-based check.

The startup harness matches 20 native/ARM/reference operation traces using
synthetic default versions, missing/present salt and missing/present mode defaults.
It intercepts converters, migrations, settings writes, commits and random input;
no real settings are touched. The deterministic salt matches the reference's
32-character output (the reference reads 17 bytes but retains only 16 in its
stored hexadecimal string). Native sanitizer checks pass. Source tests inject
failure at every operation in the salt-creation path, denied salt access, random
failure and overflowing version text. The source stops on these errors rather
than proceeding to later migration/commit steps. The source uses OpenSSL's RNG;
the tests do not assess entropy quality.

Startup can modify values before a later error and does not provide rollback.
The successful orchestration tests do not yet prove end-to-end startup with the
real converters and backend. This completes candidate implementations for the
migration entry points, but other object exports and firmware integration remain.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_init.sh
```
