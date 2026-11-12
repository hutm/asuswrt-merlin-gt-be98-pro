# Local IPC transport helpers

The experimental GT-BE98_PRO profile builds `aae_ipc.o` from this source instead
of copying the vendor object. It supplies `aae_sendIpcMsg` and
`aae_sendIpcMsgAndWaitResp` using libc socket, poll and monotonic-clock APIs.
The service daemons that receive these messages remain separate dependencies;
this replacement does not replace aaews, mastiff or their cloud functionality.

## Wire and success contracts

The ARM32 protocol sends a zero-initialized 2060-byte packet: a 2048-byte payload,
a four-byte length field, a one-byte response flag, three padding bytes and a
four-byte socket field. The caller's data is copied into the payload. Metadata
stays zero except that byte 2052 is one when a reply is requested. Compile-time
checks against the existing public header catch target layout changes. Native
tests explicitly use this wire layout independently of host `size_t`.

Both functions return one on success and zero on failure. The response helper
preserves the first successful read, including an empty read at peer closure.
It clears the supplied output capacity and copies at most capacity minus one
bytes, leaving a NUL terminator. This is not a JSON parser or a complete stream
framing implementation: a fragmented reply can still produce a partial response,
as in the existing interface. The tests deliberately cover that behavior.

## Defined failure behavior

* Requests accept zero through 2048 payload bytes. Invalid pointers/lengths,
  nonpositive response capacities/timeouts and empty paths return zero with
  EINVAL. Overlong Unix paths return ENAMETOOLONG instead of connecting to a
  silently truncated path.
* Connection readiness and blocked writes use a two-second monotonic deadline.
  Response waits use the caller's positive timeout. Interrupted calls and
  temporarily unavailable I/O retry within the deadline. Partial writes continue
  until the whole packet is sent; socket errors and zero-byte writes fail.
* SO_ERROR is checked after connecting. poll avoids the descriptor limit of
  fd_set. Sockets are close-on-exec, and cleanup preserves failure errno.
* A failed response request sets the first byte of a valid, positive-capacity
  output buffer to NUL. Other output bytes stay unchanged. Several source callers
  parse output without checking the return value; this prevents them from
  treating stale buffer contents as a response. No response data is copied on a
  timeout or read failure. Payload debug printing is omitted.

These error paths intentionally provide defined behavior rather than preserving
uninitialized-buffer copies or accepting incomplete sends. A successful return
still says nothing about the application's JSON status or authentication.

## Validation

The suite checks both exports and compares 4200 packet/response cases against
the pinned ARM vendor object. It covers payload and output boundaries, descriptor
word boundaries, immediate and asynchronous connections, response truncation and
EOF. Candidate-only tests inject 23 I/O/clock failures and retry conditions, check
a descriptor above FD_SETSIZE, reject 11 invalid-argument cases, and assert the
close-on-exec flag. Native candidate execution uses ASan/UBSan. Fortified poll/read
entry points are mocked too, so fault tests cannot fall through to real I/O.

A separate suite uses real Unix sockets between isolated processes: 26 cases
cover packet contents, one-way messages, replies, delayed and short replies,
response timeout, peer closure and a missing endpoint. It runs on ARM emulation
and native sanitizers inside non-root Docker with network access disabled.

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/aae-ipc/test.sh
```

Full-image integration and router/service interoperability remain pending.
