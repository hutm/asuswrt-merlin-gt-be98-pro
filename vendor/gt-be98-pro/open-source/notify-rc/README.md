# Service notification helpers

This source candidate supplies the ten exports of the service-notification
object: seven notification entry points, the queue-wait function, the skip
predicate, and its writable event-name pointer. It retains the existing
`validate_rc_service` provider and NVRAM/process interfaces.

Notifications validate the request, wait for the existing queue entry, publish
the request and caller PID, and signal PID 1 with SIGUSR1. The synchronous
variants additionally observe whether the queue entry changes. Default waits
are 15 seconds for `notify_rc`, 30 for `notify_rc_after_wait`, and 10/60/120 for
the synchronous variants. Custom-period variants preserve their supplied wait.

The queue-wait function uses 128-byte snapshots and the existing acknowledgment
prefix rule. It may clear an unacknowledged request at the configured threshold
(at least seven iterations). The default skippable event is `restart_autowan`.
Requests containing `reboot` retain the existing reboot-reason setting and
`killall sh` invocation before queue submission. Tests mock these operations;
they never signal processes or invoke commands on a router or the test host.

Return values preserve the existing interface: invalid notification input is
-1, a busy queue that prevents submission is 1, and ordinary submission is 0.
Synchronous return value 2 depends on the exact sequence of queue reads; it is
not a reliable generic timeout indicator. In particular, an unchanged queue
can exhaust the legacy post-decrement loop and still return 0. The candidate
preserves that behavior. Queue access remains non-atomic, and errors returned
by settings writes, signals and the reboot command remain ignored by this ABI.
Changing those contracts requires a separate caller review.

One defensive boundary differs: direct `wait_rc_service(INT_MIN, ...)` with a
busy queue returns 0 immediately instead of relying on signed wraparound.
Readable syslog diagnostics are retained; verbose console debug output is
omitted. Log text is outside the comparison contract.

The suite compares original ARM and candidate ARM outputs, then native source
execution under ASan/UBSan. It covers 8064 notification scenarios, 15360
state/failure scenarios, 20 changes-between-reads cases, maximum positive wait
with simulated completion, mutable skip configuration, rejected validation,
and a candidate-only minimum-integer boundary. Traces include settings reads
and writes, command/signal attempts and sleeps. All external effects are mocked.
This does not prove multi-process concurrency, physical storage, boot behavior,
or complete caller compatibility.

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/notify-rc/test.sh
```

Full-image and hardware validation remain pending.
