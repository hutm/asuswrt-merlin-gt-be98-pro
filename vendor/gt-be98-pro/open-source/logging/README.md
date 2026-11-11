# Source logging library

The experimental GT-BE98_PRO profile builds
`release/src/router/libasuslog/source/libasuslog.c`. Other profiles retain their
existing selection. The source exports the eight baseline symbol names and uses
libc, pthreads and the existing NVRAM library. NVRAM replacement remains separate.

Run from the repository root:

```sh
bash vendor/gt-be98-pro/open-source/logging/test.sh
```

The runner uses the local `amng-component-test` Docker image (the build image plus
qemu-user) and ARM toolchain under the sibling `asuswrt-build` directory. Override
`ASUS_COMPONENT_IMAGE` or `ASUS_BUILD_KIT` when needed. Compilation is non-root
inside Docker. It tests native code with ASan/UBSan, ARM execution under QEMU,
800-record process/thread concurrency with rotation, error handling, and isolated
Makefile source/off/other-model transitions. The isolated Makefile test uses a
minimal common.mak and mocked NVRAM dependency; it does not replace a full build.

Optionally set `ASUS_REFERENCE_LIBRARY` to an absolute baseline libasuslog.so path
for ARM differential checks. The tested baseline SHA-256 is
`61f5ef1ef563a7f9f360fa5597dc829c7cc0ef994d43a349204cca8792569104`.
The harness compares return values and captured output for file formatting,
rotation, default/custom paths, fallback, syslog priorities and console output.
NVRAM and output sinks are mocked, and timestamps/PIDs normalized. No vendor
binary is included in these tests.

Intentional differences include bounded formatting, reported flush errors, and
thread/process serialization using a stable per-UID lock file opened without
following symlinks. The baseline uses per-PID lock files. Debug behavior that
intentionally leaves a lock held is not reproduced. Exported lock helpers have
not been shown equivalent for every failure/descriptor-ownership case; the
baseline's scan of existing process locks is not reproduced. Invalid arguments
are rejected rather than reproducing crashes. No hardware validation is claimed.

A successful component test does not establish complete compatibility. The next
firmware build must verify the installed library's dependencies, consumer symbol
resolution and runtime behavior. This is one replacement within the larger goal;
wireless, acceleration, firmware and other userspace dependencies remain.
