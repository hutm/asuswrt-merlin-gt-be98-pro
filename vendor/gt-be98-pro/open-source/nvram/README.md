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
mixed implementations are not protected against concurrent commits. The ASUS
`nvram_commit` wrapper still needs its stop control and status marker. The firmware
continues to use the original main NVRAM library.
