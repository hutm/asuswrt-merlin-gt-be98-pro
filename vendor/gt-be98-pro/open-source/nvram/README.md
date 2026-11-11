# NVRAM source components

These experimental components implement the published BCM4916 kernel NVRAM
message format and supporting settings-file operations. They are not selected by
the firmware build and do not yet replace libnvram.so or libwlcsm.so.

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
| getall.h | Indexed page assembly, preserving caller output on failure |
| settings.h | Existing newline settings format and ordered loading |
| persist.h | File sync, backup/installation and recovery-file handling |

The pair test compares output with the actual helper extracted at test time from
`bcmdrivers/opensource/char/bcm_knvram/impl1/src/wlcsm_nvram.c` and its header.
Generated test material stays in the disposable container. Other tests cover
malformed messages, maximum sizes, transport deadlines and wrong senders,
empty/missing values, file parsing, and injected persistence failures.

The combined client/transport test uses a synthetic Netlink peer with state.
It exercises maximum-size SET/GET messages and an acknowledged but unapplied
update. The kernel echoes SET/UNSET requests without propagating its internal
update result: an acknowledgement does not prove that a value changed or was
persisted. Readback can observe state but cannot make concurrent updates atomic.

## Remaining integration work

The public library API still needs cache integration and synchronization, listener
lifecycle management, ASUS large-value/JFFS hooks, commit controls, locking, CLI/startup
integration and complete export coverage. The GET helper currently returns a
malloc-owned value; it must not be exposed directly as nvram_get.

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
