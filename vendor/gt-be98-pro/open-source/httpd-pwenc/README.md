# HTTP encryption and DDNS helper candidate

This source candidate supplies the four exports of the model's HTTP `pwenc.o`:
`base64`, `aes_encrypt`, `pwenc` and `gen_ddns_hostname`. It uses OpenSSL EVP/BIO
interfaces. It is not yet selected by the firmware Makefile.

The similarly named source in `aws-iot/src/pwenc.c` has different exports and
uses AES-256-ECB, so it is not a drop-in implementation of this interface.

## Behavior and bounds

* Base64 uses standard BIO line wrapping, including the final newline for
  nonempty input. The caller owns the returned allocation.
* AES uses a 128-bit key, CBC, a zero IV and zero-filled final partial blocks.
  Its legacy ABI has no output capacity: callers must supply space rounded up
  to a 16-byte boundary. Empty input writes nothing. Work is staged before the
  caller's output changes; allocation/backend failures return zero.
* `pwenc` derives its key from the MD5 of the LAN MAC string and preserves the
  legacy format's first-NUL truncation of ciphertext before Base64 encoding.
  Output is truncated to capacity minus one and terminated when capacity is
  positive. Input over 255 bytes fails without writing output; an extra internal
  terminator avoids reading beyond the rounded ciphertext. Temporary secrets
  are cleared, the encoded allocation is freed, and debug output is omitted.
  This is format compatibility, not a recommendation for new cryptographic use.
* DDNS names use the existing random-string provider with size 29, uppercase
  conversion, the final four lowercase hex digits of the string's MD5, and a
  `B` prefix. Output respects the supplied capacity. Randomness itself is not
  replaced or validated by this component.

The visible HTTP password caller is conditional on HTTPS without NVRAM_ENCRYPT;
its SMTP setting permits 128 bytes. The DDNS caller uses a 64-byte buffer.
Other binary callers, dynamic lookup and full-image bindings still need review.
Invalid pointers/negative lengths are rejected; unbounded legacy caller buffers
cannot be made safe without changing that ABI. Allocation/backend failure
injection remains untested.

## Verification

The test links the original ARM object with mocked LAN MAC/random providers and
runs it under QEMU in non-root Docker. It compares 20,363 cases against the ARM
candidate and a native ASan/UBSan candidate: 513 binary Base64 lengths, 2,048 AES
key/length combinations, 522 DDNS random-string/capacity combinations and 17,280
password/MAC/capacity combinations. EVP-based expected results and whole-buffer
canaries independently check outputs. Password reference tests stop at 239 bytes
so its 256-byte temporary always retains a terminator. Candidate-only tests
cover invalid arguments and rejection of a 256-byte input without output writes.
No real settings, radio, network service or command execution is involved.

```sh
ASUS_SPWENC_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/httpd-pwenc/test.sh
```

Passing these checks does not establish full-image or hardware compatibility.
