# GT-BE98 Pro platform helpers

Experimental source implementation of the 38 functions and one data symbol in
`private.o`, selected only for `ASUS_OPEN_COMPONENTS=y` and `GT-BE98_PRO`.
Keep model policy here and the parent Makefile hook small when rebasing upstream.
The component builds 22 source files into one relocatable object.
Hardware drivers, radio firmware and acceleration providers remain dependencies.
New implementation code is GPL version 2; public platform headers retain their
existing license terms.

## ABI and intentional corrections

`ethctl` remains a 232-byte shared request object with existing concurrency limits.
This model uses a four-argument `mdio_read` and numeric second argument for
`check_trx`. Legacy declarations describe other implementations; do not include
them in these source files.

Request parsers bound response lengths. Successful PHY writes close sockets;
skipped IE requests free allocations. CRC excludes the final 20 bytes across
read boundaries. TRX short reads and failed seeks return zero. Board searches
reject missing identity and matches extending past 16 KiB. The pointer-only
board-search ABI still requires 16 KiB of readable storage from its caller.

`check_imagefile` and `fw_check` retain existing model-specific no-op behavior.
Neither CRC nor board-name searches provide cryptographic authentication.

## Validation

Individual candidates passed comparison tests and native address/undefined
behavior sanitizers. The combined object passed 26 ARM suites. Providers are
mostly mocked; TRX and CRC also have real-file tests. The full firmware build and packaged regression passed; physical hardware
validation remains outstanding. See `validation.json` for evidence and limits.
The `test-*.c` fixtures accept a library path; some accept a second argument
for candidate-only boundary checks or corrected resource-management behavior.
Use `test_container.sh` inside a non-root `amng-component-test` container.
Mount this directory read-only at `/suite`, writable results at `/evidence`,
the ARM host toolchain at `/toolchain`, and an extracted image at `/firmware`.
Set `CANDIDATE_LIB` to the candidate library path and `REFERENCE_LIB` to the
reference library path (default `libshared.so`). The runner executes 25 comparison
suites, nine boundary suites and a CRC stream-error suite. Use a reference image
from before this replacement. Source changes also require native sanitizer
checks and a fresh full-image build and audit.
