# NVRAM source components

These experimental components implement the published BCM4916 kernel NVRAM
message format and supporting settings-file operations. The client core is not selected by
the firmware build and does not yet replace libnvram.so or libwlcsm.so. The
experimental profile now selects the source shared-library storage object;
the full-image static validation results are recorded below.

Run from the repository root:

```sh
bash vendor/gt-be98-pro/open-source/nvram/test.sh
```

The runner uses the existing local build kit and Docker component-test image,
with read-only source mounts and disposable output. Native tests use ASan/UBSan;
ARM tests run under QEMU except the Netlink tests, which are compiled for ARM
and run only on native Linux because user-mode QEMU lacks the required socket
support. No router or real storage device is accessed.

## Implemented components

| Component | Scope |
| --- | --- |
| pair.h | Bounded, unaligned-safe native-endian name/value encoding |
| frame.h | Netlink/WLCSM framing and length validation |
| transport.h | Deadline-bounded socket exchange and sender validation |
| client.h | GET, SET and UNSET request/reply validation |
| cache.h | Bounded retained-value storage and explicit refresh results |
| notification.h | Validated notification dispatch to cache invalidation |
| listener.h | Nonblocking subscription and checked receive/loss handling |
| core.c / core.h | Serialized client core with owned-copy reads and updates |
| adapter.c / adapter.h | Backend routing and borrowed-value compatibility interface |
| storage_binding.c / storage_lock.c | Shared-library hooks and compatible storage locking |
| runtime.c / runtime.h | Lazy process initialization and eight read/write ABI exports |
| shared/source/nvpriv.c | GT-BE98_PRO storage routing, migration and enumeration |
| compat/ | Bit flags, trimming, wireless-name parsing and numeric conversion |
| getall.h | Indexed page assembly, preserving caller output on failure |
| settings.h | Existing newline settings format and ordered loading |
| persist.h | File sync, backup/installation and recovery-file handling |

The pair test compares output with the actual helper extracted at test time from
`bcmdrivers/opensource/char/bcm_knvram/impl1/src/wlcsm_nvram.c` and its header.
Generated test material stays in the disposable container. Other tests cover
malformed messages, maximum sizes, transport deadlines and wrong senders,
empty/missing values, file parsing, and injected persistence failures.

The main runner also executes [compat/test.sh](compat/test.sh). Those eight
compatibility entry points are not yet linked into firmware. Their native and
ARM test transcripts match recorded reference results across 20,653 records;
the bit-flag tests also check both public aliases. Optional comparison against
a local reference library and behavior limits are described in
[compat/README.md](compat/README.md).

The combined client/transport test uses a synthetic Netlink peer with state.
It exercises maximum-size SET/GET messages and an acknowledged but unapplied
update. The kernel echoes SET/UNSET requests without propagating its internal
update result: an acknowledgement does not prove that a value changed or was
persisted. Readback can observe state but cannot make concurrent updates atomic.

## Remaining integration work

The public library still needs commit controls, persistent listener lifecycle,
CLI/startup integration and complete export coverage. The low-level GET helper
returns a malloc-owned value; the runtime uses the core's separate borrowed-value
interface for nvram_get compatibility. Real-consumer validation remains required.

The adapter now connects the core to an explicit storage-hook interface. Ordinary
keys use the kernel protocol; large keys use the storage get/set/unset callbacks
under a supplied cross-process lock. Access-policy callbacks are required rather
than silently bypassed. Enumeration appends the storage entries to kernel data
and returns the used byte count, zero-filling unused output on success. Errors
preserve the caller buffer. This improves failure handling compared with callers
that might otherwise treat a partial enumeration as complete. GETALL is still
not an atomic snapshot across backends.

`nv_core_get_borrowed` supplies the adapter's kernel-backed pointer lifetime.
The address persists until core destruction and later refreshes may overwrite
its contents. Borrowed-pointer reads require caller serialization; the core mutex
only protects operations inside the API. The owned-copy API remains available
for concurrent consumers. JFFS pointers retain the storage callback's lifetime.

Adapter tests link to the actual core/adapter shared library on native sanitizer
and ARM builds. They use a synthetic kernel peer and mock storage/policy/lock
callbacks to cover routing, missing versus empty values, retained pointers,
SET(NULL) as UNSET, positive storage-write results, denied calls, failed locks,
byte-count enumeration and output preservation. The adapter is not yet a complete
replacement for libnvram.so.

## Experimental runtime library

The runtime exports nvram_get/set/unset/getall and the four corresponding
wlcsm_nvram functions. ASUS entry points use the adapter's storage/policy routing;
wlcsm entry points access kernel storage directly. Both GETALL families return a
byte count and zero-fill unused successful output. Errors return -1 and preserve
output instead of reproducing the original kernel helper's ambiguous positive
error result. Source compatibility helpers are linked into the same candidate.

Initialization is lazy and retries after failure. The production path uses the
published BCM4916 Netlink protocol 31 with a five-second exchange timeout, opens
/var for storage locking, and caps retained cache allocations at 4 MiB. Tests
compile an explicit NV_RUNTIME_TEST variant using a synthetic exchange and a
temporary lock directory; production has no environment-variable override for
these paths. The real transport variant is ARM-compiled but not run on hardware.

An outer mutex serializes runtime operations, including storage calls. Registered
fork handlers quiesce that mutex before fork and unlock it in parent and child,
leaving private core/binding mutexes unheld. Tests cover initialization retry,
four concurrent callers and post-fork reads/writes. Forking from inside callbacks,
asynchronous signal-handler use, dynamic unloading with live state and interaction
with other libraries' fork handlers are unsupported or unvalidated. A reentry
guard reports EDEADLK for recursive NVRAM API calls rather than hanging. Shared
hook behavior must be checked for such recursion on real firmware.

The runtime retains state for process lifetime and does not run a notification
thread; every read fetches fresh kernel state. Borrowed pointers can still change
on later calls and do not become safe for concurrent readers merely because the
API operations are locked. None of the runtime or client-core candidates is
selected as libnvram.so by the firmware build yet. Commit, alternate persistence,
manager/event interfaces, remaining exports and hardware behavior remain open.

The storage binding calls the existing shared-library JFFS and access-policy
exports and supplies process/thread serialization. A local mutex serializes
threads; exclusive creation of `nvram.lock` in a caller-provided trusted `/var`
directory coordinates with the existing firmware protocol. The binding uses ten
attempts separated by 500 ms on contention. It does not replace this protocol
with flock, which would not coordinate with existing callers. The binding keeps
its own directory descriptor and requires quiescent destruction.

Lock release checks that the pathname still identifies the acquired file before
removing it. A replaced pathname is left intact and reported as ESTALE. This is
an ownership check under the trusted-directory assumption, not an atomic defense
against arbitrary concurrent pathname replacement. Stale lock files after a
process crash are not automatically removed, and fork-safe lifecycle integration
remains pending. The adapter propagates release failures: a failed SET result can
therefore follow an applied update. Callers must not infer rollback from failure.

Native sanitizer and ARM tests exercise actual temporary-file creation, forked
process exclusion, release/reacquisition, symlink refusal and replaced-path
handling. Binding tests use the real lock helper with mocked shared-library
exports to check argument/result forwarding and lifecycle. Adapter tests check
that release failures are reported and do not expose partial enumeration output.
No router filesystem or live firmware services are used by these tests.

The combined core is built as a shared library during tests. It serializes
GET/SET/UNSET/GETALL and notification handling with a mutex, refreshes every GET,
and copies each successful result into caller-owned storage before unlocking.
This allows a caller to retain its copy across concurrent updates. It does not
depend on notifications for read freshness, but still cannot provide an atomic
multi-key snapshot or a guarantee against changes after a response. Change
attempts invalidate cached state even on timeout, since the kernel may already
have applied the request. Successful change calls report only acknowledgement.

The core accepts an exchange callback that can adapt the socket transport. Its
context must outlive the core, and the callback must not reenter the same core.
Destroy requires all users to stop. Listener threads and fork-safe lifecycle
handling are not supplied. Tests link to the actual native/ARM shared library
and cover missing/empty values, unapplied acknowledgements, failed exchanges,
notification invalidation, memory limits and concurrent calls against a
synthetic peer. These tests do not establish vendor ABI compatibility or router
behavior. ASUS large-value/JFFS routing remains separate work.

The core also connects the settings loader and persistence helper.
`nv_core_load_verified` sends each parsed SET, checks the acknowledgement, then
reads back and compares the value. It reports how many entries had matching
readbacks. This is incremental: a later failure leaves earlier changes applied,
and the failing entry may also have changed. Concurrent external writers can
affect readback, and matching values do not prove durability or ownership of the
update.

`nv_core_save_at` collects GETALL and installs the existing main/_new/_old file
layout in a caller-provided trusted directory. It refuses empty replies because
the kernel cannot distinguish an empty database from an allocation failure in
that response. This prevents an ambiguous reply from replacing a nonempty saved
configuration; saving an intentionally empty database is currently unsupported.
The helper returns the persistence result, including a distinct uncertain-sync
status after installation. It still requires external cross-process writer
serialization, and collection across pages is not atomic. ASUS commit controls,
post-save notification, startup recovery selection and flash validation remain
required before exposing this as nvram_commit.

Shared-library tests on native and ARM builds now cover ordered loads, empty
values, acknowledged-but-unapplied SET rejection, partial load failures, save
backup contents, and preservation of the existing file on transport failure or
ambiguous empty collection. Persistence failure-injection tests continue to cover
write, rename, rollback and sync failures separately. All files are disposable;
these are not router power-loss or flash-storage tests.

The cache reserves a 1024-byte value buffer for each stored key so growth does
not relocate an exposed pointer. It never evicts entries, applies a caller-set
memory limit, and reports exhaustion without changing prior data. A confirmed
missing value clears the retained buffer and returns NULL; an empty value returns
a non-NULL pointer. Refresh errors preserve previous data but return an error,
so callers must explicitly decide whether stale data is usable. Returned pointers
remain allocated until cache destruction. All access, including reads through
retained pointers, requires external serialization; this helper alone does not
make concurrent callers safe.

This deliberately differs from the reference library's growth behavior observed
with a synthetic response peer: a short update reused the pointer, while a
600-byte update returned the old value once and a new pointer on the next read.
The source cache applies a valid growth update immediately. Compatibility with
real consumers still needs evaluation. Cache tests cover address retention,
growth, aliases, missing/empty distinctions, memory limits, refresh errors and
multiple keys on native sanitizer builds and ARM emulation.

Notifications mark values stale instead of accepting the broadcast payload as
applied state. The kernel broadcasts requests without propagating update errors.
An invalidation preserves retained storage, while new cache lookups return
EAGAIN until refreshed. A generation change during a fetch rejects its result;
this conservatively includes unrelated-key events. This check assumes externally
serialized state access and does not itself implement concurrent locking.

The persistent listener subscribes to multicast group 1 and checks recvmsg sender
metadata for kernel port 0 and group 1. The header pid is the initiating process
and is not used as authentication. Truncation, malformed trusted frames and
receive loss invalidate all cached entries; EINTR and EAGAIN do not. The listener
must subscribe before initial cache population, and reopening after a gap must
invalidate existing state. Notification loss that the kernel never reports is
not detectable, so freshness still requires an explicit refresh policy.

Notification decoding and listener error handling pass native sanitizer and ARM
tests. Listener socket operations in these tests are mocked; the existing real
usersock tests exercise the separate request transport. Actual vendor multicast
delivery, event-loop/thread shutdown, fork behavior and on-router synchronization
remain untested. None of these components is yet the installed library.

The transport option for the vendor kernel's recipient-as-sender convention is
based on its source and remains untested on hardware. The protocol has no
reliable sequence identifier. Random fresh socket IDs reduce delayed-reply reuse
but do not prove correlation across arbitrarily many exchanges. GETALL pages
are not an atomic snapshot, and empty completion also covers a kernel allocation
failure that the protocol does not distinguish.

Persistence requires a trusted directory descriptor and externally serialized
writers. It preserves existing recovery files and reports uncertain durability
separately when the final directory sync fails. Power-loss recovery, actual flash
behavior and firmware reader permissions need validation. New settings files use
mode 0600. The existing newline format cannot faithfully round-trip embedded LF
values; oversized physical input lines are rejected rather than split into
multiple entries. There is no transaction across incremental loaded updates.

## Large-value storage hooks

The source storage implementation includes large_nvram, large_nvram_t,
jffs_nvram_get, jffs_nvram_set and jffs_nvram_unset. Its GT-BE98_PRO table has
304 exact-match identifiers and two migration exceptions. Routing requires the
/jffs directory and /jffs/nvram_war marker. The reference object and candidate
passed matching ARM tests for every identifier and both marker gates, unknown
keys and suffixes, missing versus empty files, helper errors and return values.
The candidate also passes native sanitizer tests. All filesystem helpers are
mocked in these tests; no JFFS device is accessed.

Reads preserve the existing shared 16 KiB buffer behavior, including returning
an empty string on a nonpositive file-read result. Writes return the underlying
f_write_string result, which can be a positive byte count; NULL means unset.
These semantics must be accounted for when connecting them to the core. Calls
require external serialization. Invalid names, path separators and oversized
paths are rejected instead of traversing directories or silently truncating.

The source also implements internal integer lookup, startup migration and
combined enumeration. Common behavior tests pass against the reference object:
ordinary values migrate then clear the kernel entry, while cfg_relist and
cfg_relist_x use their _t flags and retain the original value. A commit is
requested after changes. Enumeration preserves ordering and the strict extra-byte
capacity rule, skipping entries that do not fit. Caller buffer suffix bytes are
not automatically cleared. Unlike the reference, migration stops on a failed or
short file write before clearing that kernel entry. Partial migration remains
possible; the existing void API provides no structured error result.

Sources live in release/src/router/shared/source. The shared-library Makefile
selects them only for ASUS_OPEN_COMPONENTS=y with GT-BE98_PRO and rebuilds the
object when switching profiles. Native sanitizer and ARM tests pass. Full-image
linking and packaged-library checks passed; hardware storage tests remain pending.
The table is specific to this model and input version and must be checked again
when changing the ASUS baseline.

## Full-image storage build

Commit `b1d49a0b33` built successfully on 2026-11-11. The normal image is
76,719,844 bytes with SHA-256
`92e80003ec7b54e4dcbeb94ecc0b4f75b184eb5a63a9017b3a6480773b240756`.
Its packaged libshared.so exactly matches the stripped build library and differs
from the prior image. The compiled source storage object exports all eight
expected functions; their sizes match the packaged library's dynamic symbols.
The packaged library SHA-256 is
`5f9b8d58a0fb585d0ced5ff094768f072dfca48c6fa891128ecca2b8b22ebfb4`.

The userspace scan covered 647 ELF objects with no new or changed dependency
flags relative to the baseline's 81 standalone/plugin-scope flags. All 151 kernel
modules passed the available static checks against the matching packaged kernel.
FIT payload integrity passed. These results do not establish hardware behavior,
private ABI compatibility or complete provenance. Other binary objects still
contribute to libshared.so, and libnvram.so remains the original implementation.

## Internal commit transaction

`nv_core_commit_at` saves the kernel settings and then sends the type-7 commit
notification while holding the core mutex. The kernel handler only echoes and
broadcasts that message; it does not write settings. External cross-process
writer serialization remains required. Return values distinguish pre-install
failure (-1), installed but final directory sync failed (-2), and installed and
synced but notification unconfirmed (-3). Notification failure does not roll back
the saved file, and a timeout may still mean the notification was delivered.

Native sanitizer and ARM/QEMU tests cover successful notification, timeout, wrong
message type, nonempty and oversized replies, retained settings after notification
failure, and suppression of notification after empty enumeration or an existing
staging file. The lower-level persistence tests cover write/sync/rename failures.
The experimental runtime now exports `wlcsm_nvram_commit`, taking the storage
lock around the save and notification, opening `/data` directly, and mapping
internal failures to the vendor ABI's return value of one. It releases the lock
on every exit and reports release failures. Errors can follow successful writes;
callers must not interpret a nonzero return as rollback. The 262144-byte snapshot
limit matches the reference. There is no environment-based path override.

Runtime tests cover missing data directory, occupied staging path, successful
commit, notification timeout after persistence, lock cleanup and a subsequent
successful retry. Native sanitizer and ARM/QEMU tests pass. Source writers share
the exclusive storage lock, but original-library committers need not acquire it;
mixed implementations are not protected against concurrent commits. The firmware continues to use the original main NVRAM library.

The runtime also exports `nvram_commit`. Any present `asus_stop_commit` value,
including empty or `"0"`, suppresses the operation. Otherwise it commits, calls
`sync()` even after commit failure, and on success writes `commit: OK\n` to
`/var/log/commit_ret`. Marker errors remain best-effort, matching the reference.
An ARM reference-library test confirmed presence-based suppression, commit/sync
ordering and success-only marker creation. Runtime sanitizer and ARM/QEMU tests
also cover marker contents, marker-open failure, notification failure and retry.
Unlike the reference's ambiguous NULL getter, a denied or failed control read
fails the operation rather than proceeding with a write. The control is read
from the kernel backend, consistent with this model's storage routing table.
No hardware persistence or power-loss testing has been performed.

## Salt accessor

The runtime exports `nvram_get_salt` for the shared-library consumer. It checks
the existing program policy and reads the kernel `nvram_salt` key directly.
The ABI returns a borrowed string, NULL for missing/error, and `(char *)-1` when
access is denied; callers must check the sentinel before dereferencing. This
function retrieves an existing value and does not generate or log salt material.

An ARM reference-library test with synthetic values confirmed the key, policy
gating, denied sentinel, and missing/empty/borrowed return behavior. Native
sanitizer and ARM/QEMU runtime tests also cover transport failure. This adds
another required API but does not enable the replacement library in firmware.

## Direct kernel setter

`nvram_kset` and `common_nvram_kernel_set` use the serialized core SET operation
without ASUS JFFS routing or getter policy. NULL is a zero-length SET value,
whereas the ordinary runtime setter maps NULL to UNSET. The ARM reference test
confirmed the message type, payload for present/empty/NULL values, invalid-key
return and send-error return. Source native sanitizer and ARM/QEMU tests cover
these cases, direct-backend behavior and transport failure. The source checks
the echoed payload as well as message type; acknowledgement still does not prove
the kernel applied the change.

The runtime now implements all names identified by the earlier firmware
dynamic-symbol consumer scan. That scan does not
cover dynamically constructed symbol lookups, static consumers or every export
of the original library. Implementing those names alone is insufficient to
claim full library compatibility or hardware readiness.

## Kernel-only commit

The runtime exports `nvram_kcommit`, saving the kernel snapshot to
`/data/.kernel_nvram.setting` without ASUS suppression, a status marker, global
`sync()` or a type-7 notification. Its ABI returns zero on success and -1 on
failure. An ARM reference test confirmed the path, kernel enumeration callback,
newline serialization and init/save failure returns.

The source deliberately uses the existing recovery-file transaction and exclusive
storage lock instead of the reference's in-place truncate and file-record lock.
It preserves live settings on enumeration failure, refuses an occupied staging
file and can create a missing live file when the data directory exists. It does
not interoperate as a writer lock with original in-place committers: mixed-library
concurrent writes remain unsupported. Error returns can follow file installation.

Native sanitizer and ARM/QEMU runtime tests cover successful save, bypass of ASUS
stop/policy controls, absence of notification/sync/marker calls, failed enumeration
with preserved settings, unavailable data directory, occupied staging, lock cleanup
and retry. Hardware persistence testing and replacement-library integration remain
pending.

## Character-set conversion transport

`nvram_xfr` and `wlcsm_nvram_xfr` send a type-14 request to the kernel NLS handler,
whose source is in bcm_knvram's wlcsm_netlink.c. The FTP filename conversion
consumer uses this interface. Inputs are limited to 511 bytes plus the terminator.
The result is a shared borrowed buffer overwritten by a later successful call;
empty replies and errors return NULL. Embedded NUL terminates the result, and a
non-NUL-terminated kernel payload receives a bounded terminator in userspace.

ARM reference tests confirm request framing, input boundaries, shared-pointer
behavior, embedded-NUL handling and missing/error replies. Native sanitizer and
ARM/QEMU tests additionally cover maximum replies, oversized or wrong-type
frames, transport errors and unchanged output on failure. Both exported runtime
names are exercised. These tests use synthetic peer responses; actual kernel NLS
conversion and hardware behavior have not been exercised. The source refuses
NULL inputs and oversized replies rather than reproducing unbounded accesses.

## Manager restart protocol work

`manager.h` encodes the restart request as manager GET type 9 with the manager
routing flag, a 20-byte variable header and the `restart` name/value pair. It
preserves the reference's packed argument behavior while initializing reserved
bytes. An ARM reference-library harness checked 392 argument/return cases against
the source encoder. The kernel source routes this request through its registered
wireless manager, which returns type-10 fragments and a type-11 final fragment.

The source accumulator bounds responses to the reference restart buffer's 16-byte
capacity, rejects empty or wrong-type fragments, and requires a consistent nonzero
manager ID. It preserves state on error. Transport-origin validation is still
required; the inner manager ID is not authentication by itself. Native sanitizer
and ARM/QEMU tests cover framing, accumulation, ID changes and overflow.

The runtime now exports `wlcsm_mngr_restart`. The transport keeps one socket
open through the multipart exchange with a deadline covering send and all
receives. Every fragment passes sender validation before decoding. Single-reply
callers use the same transport with no continuation callback. The runtime maps
completion to zero and errors to one; a response acknowledges the exchange, not
successful hardware restart. A timeout may follow an applied restart.

Native Netlink usersock tests cover two-fragment completion, incomplete-response
timeout, wrong-sender final replies, oversized replies and changing manager IDs.
Runtime native sanitizer and ARM/QEMU tests cover exported success and failure.
The real transport path also cross-compiles for ARM; Netlink usersock tests run
natively because QEMU does not provide that facility. No restart was sent to a
router or running wireless manager. Actual manager/hardware validation and full
replacement-library integration remain outstanding.

## Experimental firmware selection

The GT-BE98 Pro WLAN Makefiles now select this source library when
`ASUS_OPEN_COMPONENTS=y` and `BUILD_NAME=GT-BE98_PRO`. Both normal and prebuilt
WLAN build paths invoke the source library target. Default profiles retain their
existing selection. The library is rebuilt on each invocation to prevent a
previously copied vendor library from satisfying the source target by timestamp.
The command-line `nvram` executable remains the existing prebuilt artifact.

The standalone Makefile builds an ARM `libnvram.so` with an explicit SONAME and
export map, hiding internal implementation symbols. Its 25 public exports cover
all 18 names in the earlier consumer scan. Production compilation against the
built target libshared.so passed; dependencies are libshared, pthreads, libc and
the ARM loader. This is API-name coverage, not full ABI or behavior validation.
The original library has additional exports not implemented here.

Full-image linking, final packaged-library identity, runtime boot settings loading
and hardware validation must still be checked before treating this selection as
a working firmware replacement. No installable release is implied by this build
selection.

## First full-image source-library build

Commit `ab5242cf43` completed the experimental GT-BE98 Pro firmware build on
2026-11-11. The normal image is 76,715,748 bytes with SHA-256
`52b263378ab56427cfdef5672ab82340fcf4f09cc988cce5507f43fc0ad23af0`.
The extracted `/lib/libnvram.so` is byte-identical to the stripped source build;
its SHA-256 is
`6bf0184eb6db1b126e2190e0dba0a7c48484f7660e4d81e14a30d8a7111f1508`.
The final-image userspace scan covered 647 ELF objects with no new or changed
unresolved-dependency flags relative to the previous image. FIT payload hashes
passed. A separate production-library load test against the preceding firmware
libraries passed under ARM emulation with eager symbol resolution.

This confirms packaging of the source library, not boot or hardware behavior.
The CLI is still prebuilt. Its replacement must preserve `kernelset`, `kget`,
`kset`, `kunset`, `kcommit` and manufacturing restore behavior used by packaged
scripts. A reference test shows `restore_mfg` opens `/mnt/nvram/nvram.nvm`; it is
not simply the newline settings loader. Other proprietary userspace, kernel,
radio and boot components remain. This image is experimental and unvalidated on
router hardware.

## Source CLI kernel-command adapter

`cli_kernel.c` implements command dispatch for `kget`, `kset`, `kunset`,
`kcommit` and `kernelset`, ready for integration with the existing GPL CLI.
Reference CLI runs with synthetic backend stubs established that `kget` calls
the ASUS getter, `kset` calls the ASUS setter followed by the direct kernel
setter, and `kunset` calls ASUS unset followed by a NULL kernel SET. `kcommit`
checks presence of `asus_stop_commit` before kernel-only persistence.

The adapter preserves those sequences and reports missing arguments, file errors
and backend failures. Unlike the reference it stops the second write if the first
fails, and opens input settings read-only. File loading uses the already tested
bounded newline parser, preserves duplicate ordering and stops on failure with
possible partial updates. The legacy pointer-only getter still cannot reliably
distinguish missing values from read errors; this ambiguity remains for `kget`
and the CLI `kcommit` control check.

Native sanitizer and ARM/QEMU tests cover routing, NULL values, first-write
failure, commit suppression, settings loading and invalid input. This adapter is
not yet selected as the firmware CLI: manufacturing restore and the rest of the
existing CLI integration remain outstanding. The packaged CLI is still prebuilt.

## Manufacturing settings conversion

`manufacturing.c` converts the NUL-separated manufacturing records into newline
settings, preserving order and appending to the supplied output stream. An empty
record terminates input. If no record begins with `et0macaddr=`, an optional
base-MAC stream supplies up to 18 bytes for the added setting. The reference CLI
uses `/mnt/nvram/nvram.nvm` as input, opens the command's destination in append
mode and consults `/proc/nvram/BaseMacAddr` for that fallback.

An isolated ARM reference CLI test confirmed append behavior, termination at an
empty record and the MAC-present/MAC-absent cases using synthetic files. Native
sanitizer and ARM/QEMU source tests match those output bytes. The source also
rejects overlong records and unterminated final records instead of relying on
unbounded string reads. It reports I/O errors; partial output remains possible.
This stream converter still needs the CLI file-opening/cleanup integration and
full CLI selection. No manufacturing data from router hardware was read or changed.

## GPL CLI integration candidate

The existing `release/src/router/nvram/nvram.c` now has an opt-in
`ASUS_OPEN_NVRAM_CLI` adapter hook. Kernel commands are consumed before the
legacy parser, including chained commands. This mode uses the GT-BE98 Pro
262144-byte settings capacity instead of the legacy `get_nvram_space` backend,
which is unavailable in this model's runtime library. Other builds keep their
existing path. Manufacturing restore opens the fixed manufacturing source and
optional base-MAC source, appends to the requested destination, checks close and
flush errors, and fsyncs successful output. Partial writes are not rolled back;
concurrent callers must serialize destination writes.

File-level native sanitizer and ARM/QEMU tests cover append, MAC fallback,
missing sources, invalid destinations and malformed records. The adapted GPL
CLI cross-compiled and linked against the source-library firmware's libraries;
its usage path executed under ARM emulation. A synthetic-backend integration
run verified chained `kset`, `kunset`, `kcommit` and `kernelset` operations and
matched the earlier captured settings sequence. The candidate is not selected
in firmware yet: remaining command parity and full-image integration still need
validation. The existing GPL CLI's backup/restore and erase behavior also need
review before replacing the packaged executable.

## Bounded profile restore candidate

The opt-in GPL CLI uses `profile.c` for HDR1/HDR2 restore. It limits declared
payload length to the allocated settings capacity, rejects truncated files,
decodes HDR2 within that bound, and validates string termination before applying
any records. Setter errors stop with possible partial changes. Invalid or
unterminated input no longer reaches the legacy unbounded buffer scan. A record
without `=` unsets its complete key once rather than repeatedly scanning suffixes.
Default CLI builds keep their existing implementation.

Native sanitizer and ARM/QEMU tests cover both formats, oversized/truncated data,
unterminated records and callback failure. The complete adapted CLI cross-links.
The initial short HDR1 mismatch was traced to empty parameter tables in the
reference stubs plus a filter difference in the older GPL CLI. The opt-in source
filter now consults `router_prefix_defaults` and `router_defaults`, matching the
reference table ordering and the `wl0_failed` exclusion. With populated synthetic
tables, HDR1 and HDR2 restore produce identical setter transcripts, including
allowed prefixes, ordinary table entries, unknown-key rejection and that exclusion.
This does not yet establish compatibility for all backup files. The reference usage also identifies missing `getflag`, `setflag`,
`loadfile`, `savefile`, `dump` and restart command variants for the parity audit.


Run the complete CLI profile comparison with
`ASUS_CLI_REFERENCE_ROOT=/absolute/extracted/root bash vendor/gt-be98-pro/open-source/nvram/test_cli_profile.sh`.
It compiles the adapted GPL CLI and synthetic backend stubs in non-root Docker,
runs both executables under ARM emulation and checks their output against each
other and an explicit expected transcript. It does not write settings to a router.

## Flag commands and text loading

The CLI adapter now supports `getflag name bit`, `setflag name bit=value` and
`loadfile [path]`. Flag commands route to the existing WLCSM helpers and preserve
the reference's value-before/value-after output. They reject nondecimal indices,
indices outside 0..31 and overflowing values rather than relying on `atoi`.
Inputs remain unmodified. Getter failure retains the existing missing-value
ambiguity. Native sanitizer and ARM/QEMU tests cover dispatch, output, invalid
arguments and setter failure. The linked CLI reference comparison includes both
flag commands with synthetic backends and matches their transcripts.

`loadfile` uses the direct WLCSM setter; `kernelset` continues to use the direct
kernel setter. Omitted loadfile paths select `/data/nvramdefault.txt`, matching
the reference. The source opens files read-only and reports parse/setter errors.
Tests distinguish the two setter routes and preserve duplicate/empty values.
Dump variants and remaining ordinary-command error handling
still need work before selecting the source CLI in firmware.

## Text export candidate

`savefile [path]` now exports the ASUS getter's NUL-separated settings as text,
retaining record order, empty values and embedded equals signs. The default path
is `/data/nvramdefault.txt`. The complete CLI's exported bytes and success message
match the reference executable under ARM/QEMU with a synthetic settings backend.
This is a text export, distinct from the HDR1/HDR2 backup commands.

The source validates the complete snapshot within 262144 bytes before opening
its destination. Getter failures and missing record/list termination therefore
leave an existing export untouched. Native sanitizer and ARM tests cover these
cases and invalid destination paths; the full component suite passes. Output
and close failures are reported, but errors after opening may leave a partial
file. This change does not select the CLI in the firmware or establish hardware
compatibility.

## Commit and wireless restart commands

The opt-in CLI implements `restart`, `commit` and `commit restart`. The standalone
restart requests manager arguments `(0, 1, 0, 0)`. Commit checks the presence of
`asus_stop_commit`, calls the WLCSM commit, then saves kernel settings. Commit
with restart instead requests `(0, 1, 1, 0)` before saving kernel settings. ARM
comparisons of the complete source and reference CLIs with synthetic backends
confirm these call sequences. A manager acknowledgement is not evidence that
wireless hardware restarted successfully.

The source stops and reports a failure if either step fails; it cannot roll back
an earlier completed step. Native sanitizer and ARM tests cover both sequences,
manager failure, commit failure, kernel-save failure, and stop values including
empty and `0`. As with the existing kcommit adapter, the pointer-only stop getter
cannot distinguish a missing value from a read failure. This remains a limitation.
The source accepts only the exact optional `restart` argument and preserves any
other following command, unlike the reference's prefix match and consumption of
an unrelated argument. All component tests and the CLI reference comparison pass.
The source CLI is still not selected in firmware; dump filtering, backup writing,
erase and remaining ordinary-command behavior need review.

## Filtered settings display

The source CLI handles exact `show`, `dump` and `getall` commands. All three
retrieve the ASUS settings view, apply `invalid_nvram_get_name` to each key,
preserve record order, and report used/remaining bytes including the 20-byte
header allowance. Positive getter lengths can span empty slots: those slots
produce the reference's `XX ILLEGAL nvram` marker if the policy permits them.
The complete source and reference CLIs match stdout and stderr under ARM/QEMU
for a fixture containing restricted settings, empty slots and ordinary values.

The source validates all record boundaries and the reported length before any
output; getter errors and malformed or unterminated snapshots fail without
printing partial settings. Native sanitizer and ARM tests also cover a full
buffer, an out-of-range reported length, and output failure. The policy receives
the complete key; unlike the reference, the source does not truncate names to
127 bytes before checking them. A long restricted-key test confirms that its
value is withheld. This delegates policy to the existing shared library, whose
remaining proprietary components still require replacement. Firmware selection
of the source CLI awaits backup/erase and ordinary-command review.
