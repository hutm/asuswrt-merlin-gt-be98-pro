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

## Combined credential lifecycle validation

`test_lifecycle.sh` links all source password, settings, migration and startup
components together. Only the settings backend, device identity and entropy are
substituted with synthetic fixtures. The reference run uses the original
library's complete credential path against the same in-memory backend. Native
runs include address/undefined-behavior sanitizers; ARM runs use the extracted
firmware libraries under QEMU.

Eight scenarios cover default versions 2/3, missing/existing salt, and empty/
nonempty password and account-list credentials. Each initializes twice, for 16
compared final states. Every stored value and presence flag matches the reference.
The tests also assert decoded credentials, account-list ordering, recovery of a
formerly encrypted setting, preservation of an ordinary setting, final mode and
version, and one commit per initialization. The second initialization leaves all
stored values unchanged. Version 3 with initially missing salt preserves the
credentials while creating the deterministic test salt.

This closes the earlier successful-path integration gap between the credential
candidates. It does not validate the production NVRAM backend, concurrent writes,
crash recovery, real entropy, other default versions or the full firmware build.
Other exports from `spwenc.o` still prevent selecting its complete replacement.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_lifecycle.sh
```

## Settings access-check candidates

`access.c` provides `invalid_nvram_get_t`, `invalid_nvram_get_name`,
`invalid_program_check` and `invalid_nvram_get_program`. The 20 protected names
and NULL exclusion fields match the original table. Name matching is exact.
`nvram_salt` is always denied through the general program check. Other protected
names are allowed for executable paths beginning with `/usr/`, `/bin/` or
`/sbin/`; ordinary names bypass executable inspection. NULL names return 0.
This preserves a path-based compatibility policy, not a general authentication
or executable-integrity guarantee.

The source denies protected-name access if `/proc/self/exe` cannot be read or the
127-byte path read may be truncated. This intentionally differs from the
reference's permissive read-error behavior and prefix handling for truncated
paths. It also omits the reference debug message for rejected paths.

The suite compares all table entries and 322 native/ARM/reference name/path
cases, including NULL names, near-match names, directory boundaries and ordinary
names. It compares readlink-call counts as well as results. Native sanitizer runs
and source-only missing/truncated identity tests pass. Executable paths are
intercepted in the harness; these tests do not establish enforcement by every
settings consumer. These candidates remain outside the firmware build pending
the remaining shared-object interfaces.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_access.sh
```

## Privacy-policy query candidates

`policy.c` supplies `ASUS_PP_t` and the three `get_ASUS_privacy_policy*` queries.
All eight names, required versions, IDs and force-sign flags match the reference.
The state query retrieves the setting named by `webapi_get_b(3)`, copies at most
31 characters, and compares its nonzero decimal value with the policy's required
version. Unknown IDs return 0. The value query delegates to `nvram_get_int` after
a successful setting-name lookup. These functions only read settings; they do
not grant consent or change stored policy state.

The suite matches native sanitizer, ARM and reference output for the table, 60
value queries and 720 state/version combinations. Cases include missing/empty
settings, threshold values, negative and malformed numbers, numeric suffixes,
32-bit overflow and 31-character truncation. Settings-read counts are checked for
successful and failed name lookup. The name lookup and settings backend are
intercepted; their production integration is not established by these tests.
The source explicitly terminates the lookup buffer and uses the ARM numeric
range on native hosts. Policy initialization, timestamp updates and the underlying
name-lookup helper still require replacement before this object can be removed.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_policy.sh
```

## Settings-name lookup and character append candidates

`names.c` supplies `webapi_get_b` as a direct eleven-entry mapping. Valid IDs
return the copied name length, truncated to destination capacity minus one;
unknown IDs clear the supplied destination and return 0. NULL or zero-capacity
buffers return 0 without writing. The exported `c` helper appends promoted
character values until a zero low byte, preserving bounded truncation. The source
also refuses to append to a buffer without a terminator inside its capacity.

The lookup suite matches 495 native/ARM/reference cases (IDs -1..13, capacities
1..33), plus 13 append cases including truncated and low-byte character values.
NULL destinations match; zero-capacity and unterminated buffers have source-only
checks. Native sanitizer runs pass. `test_policy_names.sh` then links the real
source lookup to the policy queries and compares 15 value and 180 state/version
cases against the original complete path, using synthetic settings only. The
original mocked lookup suite remains available to exercise lookup failures.
These functions remain outside firmware until the other object exports are ready.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_names.sh
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_policy_names.sh
```

## NTP timestamp update candidate

`timestamps.c` supplies `update_ntp_ts`. It skips updates when `ntp_ready` equals
1; otherwise it examines the EULA and policy timestamps, updating nonzero values
older than the supplied pre-adjustment time. It writes both the adjusted decimal
value and the corresponding `rfctime` string, using the source settings-name
lookup for the policy keys. No commit is performed.

For compatibility, input and stored decimal output use the reference's ten-
character limit. Parsing, signed comparisons and addition reproduce ARM 32-bit
behavior using explicit unsigned arithmetic and bit conversion, avoiding signed
C overflow. This includes legacy wraparound and truncated negative output; the
function is not a redesign of timestamp storage for dates beyond that range.
Like the original void interface, settings-write results are not propagated.

The suite compares 576 native/ARM/reference update traces across readiness
values 0/1/2, sixteen stored-value forms, three comparison times and four time
offsets. Cases include missing/empty values, malformed numeric text, threshold
boundaries, truncation, negative values and signed/unsigned 32-bit boundaries.
Native sanitizer checks pass. Settings writes and `rfctime` are intercepted, so
the evidence proves update decisions and formatter inputs, not production time
formatting or persistence. Policy initialization remains to be implemented; this
candidate is not yet selected in firmware.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_timestamps.sh
```

## Policy timestamp initialization candidate

`policy_init.c` supplies `init_asus_pp_eula`. It snapshots the EULA timestamp,
loads the EULA version (using 1 when that value is zero), and wraps a nonempty
ASCII digit-only timestamp as `<version>timestamp`. It then applies the same
normalization to the policy timestamp using the policy version, including zero.
Already structured timestamps and other nondigit values are unchanged. Version
text retains at most seven characters, input snapshots at most 1023, and output
records at most 1023, matching the original limits. The function does not commit
settings and, like the original void interface, does not propagate write errors.

The suite links the real source settings-name lookup and compares 119 native/
ARM/reference write traces: 112 combinations of timestamp forms and versions,
plus seven long-value boundaries through 1100 input characters. Cases cover
missing/empty values, leading zeros, signs/whitespace, existing records, malformed
text, negative versions and version truncation. Native sanitizer checks pass.
Reads/writes use synthetic fixtures; persistence, concurrent changes and backend
errors are not covered. The source policy initialization and timestamp exports
are now candidates, but the full object still requires its remaining interfaces
and firmware integration before removal.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_policy_init.sh
```

## Legacy MD5 context adapter

`md5_abi.c` supplies `MD5Init`, `MD5Update` and `MD5Final` with the original
88-byte layout: four state words, two bit-count words and a 64-byte buffer. The
adapter uses OpenSSL's block transform and implements buffering/counting around
it. It preserves the original untouched buffer bytes after initialization and
partial updates, and clears the complete context on finalization. There are no
heap allocations or hidden pointers in the caller's context.

This adapter deliberately uses OpenSSL's deprecated low-level `MD5_Transform`
API to preserve callers' inspectable/copyable context state. The pinned OpenSSL
build supplies that symbol; an upgrade that removes deprecated APIs will require
an alternative transform backend. The higher-level password candidate continues
to use EVP. MD5 here preserves existing interfaces and formats, not a new security
design. Context/input pointers must satisfy the existing ABI contract.

The suite compares full context bytes and final digests for 4626 native/ARM/
reference cases: lengths 0..513 and nine chunk sizes spanning padding/block
boundaries. It additionally checks bit-counter rollover, zero-length updates and
complete context clearing. Native address/undefined-behavior sanitizers pass.
This adapter remains outside firmware pending the remaining object interfaces
and integration checks.

```
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/spwenc/test_md5_abi.sh
```
