# GT-BE98 Pro complete source replacement experiment

Branch: gt-be98-pro/open-source. Baseline: 9a1d8889be. Separate sparse worktree
keeps the successful ASUS-input build untouched. This is not a blob-free image.
The objective covers **all** final-image proprietary components: userspace,
kernel modules, embedded radio/accelerator firmware, and boot-stage components.
Interface analysis alone does not establish source completeness, licensing rights,
or equivalent behavior. Do not relabel proprietary disassembly as open source.

## First implemented change

`ASUS_OPEN_COMPONENTS=y` selects the existing open filesystem build paths for
GT-BE98_PRO: TFAT=n, NTFS="open", HFS="open". It is opt-in and leaves other
model profiles unchanged. Traced through src-rt/Makefile, router/Makefile and
rc/usb.c: these select FAT/VFAT, NTFS-3G and kernel HFS/HFS+ instead of the Tuxera
modules. This is a configuration change, not yet a validated removal from an image.
Use a fresh configuration/build: old RTCONFIG_* values and staged modules could
survive incremental rebuilds. The iteration launcher forwards this flag when explicitly set in its environment.
Default builds do not enable it. Kernel configuration now explicitly requests
FAT/VFAT and HFS/HFS+ modules plus built-in FUSE for this profile.

Outstanding: actual kernel FUSE/FAT/HFS configuration; compile NTFS-3G and filesystem
tools; final rootfs absence of Tuxera modules/tools; FAT/NTFS/HFS read/write tests;
NTFS source version/security review; disk-format UI/tool compatibility; journaled
HFS+ limitations. Preserve filesystem support rather than counting feature deletion
as equivalent replacement. NTFS3 and newer exFAT drivers are alternatives, but a
Linux 4.19 backport is additional work, not a drop-in library substitution.

Sources checked:
- https://www.kernel.org/doc/html/latest/filesystems/ntfs3.html
- https://www.kernel.org/doc/html/latest/filesystems/exfat.html
- https://wireless.docs.kernel.org/en/latest/en/users/drivers/brcm80211.html

## Full remaining scope

| Component group | Direction | Current evidence/status |
| --- | --- | --- |
| Tuxera FAT/NTFS/HFS | Existing source implementations | Opt-in selection implemented; no image validation |
| sw_gso | Reconstruct missing queue worker implementation | Partial enqueue model, not a driver |
| Trend Micro tdts/udb + userspace | Evaluate source inspection engine and service/UI adaptation | No drop-in replacement established |
| Broadcom Ethernet/switch, packet acceleration, RDPA/BDMF | Trace actual source coverage and hardware interfaces | Module baseline and input graph available, incomplete |
| Broadcom wireless wl/dhd/hnd + radio firmware | Find chip-specific open driver/firmware or reconstruct | No verified GT-BE98 Pro replacement; open host driver alone is insufficient |
| ASUS private userspace objects/libraries | Inventory linked functionality, find source alternatives, reconstruct APIs | 280 pinned inputs are an input list, not a complete functionality inventory |
| Boot stages, embedded microcode, auxiliary firmware | Trace build inputs and extract payload inventory | Source completeness not established |
| Other prebuilts with source available | Rebuild from identified source | Needs per-input attribution; GPL labels alone insufficient |

Keep a final-image inventory as the acceptance evidence; no category can disappear
from scope because it is difficult. Optional removals must be reported as feature
loss, never as completed equivalents. Hardware tests require recovery readiness.

## sw_gso partial reconstruction

Evidence is the standard baseline module (not a claim of equivalence to every ASUS
release), SHA256 7fba345b7959ab34f7a0d39cafb572d1b8ecd58dc2f1cf8d2ee040e2ac8aeb52.
Local reference evidence is kept outside Git.

New observations from sw_gso_enqueue:
- 0x2dc returns **positive 16**, leaving the packet to the caller when unavailable.
- 0x210–0x238 uses 16-bit producer/consumer counters; a full ring or inactive worker
  goes to 0x408, unlocks, frees the skb and returns **zero**. Treating zero as proof
  of successful transmission would be incorrect.
- 0x25c–0x268 writes device, packet and two callback pointers into a 32-byte slot.
- 0x26c publishes through a store barrier before advancing the 16-bit producer.
- 0x294–0x2b0 uses atomic compare/exchange for worker wakeup.

sw_gso_model.py captures only the observed enqueue ownership and indexing contract.
Its consumer is explicitly a test harness. Tests exercise full queues, inactive
queues, unavailable workers and 16-bit counter wrap. They do not establish threaded
correctness or binary equivalence. Next reconstruct worker batching, callback
contracts, requeue handling, initialization/failure paths, shutdown and flow rehash.

Run: `python3 -m unittest discover -s vendor/gt-be98-pro/open-source -p 'test_*.py'`.

## Component build evidence

NTFS-3G 2022.10.3 from the existing repository sources compiled successfully inside
amng-build as a non-root user with the ARM soft-float GCC 10.3 toolchain. The
result is an ARM EABI5 executable using /lib/ld-linux.so.3. Internal FUSE and
libntfs-3g were compiled from source and linked into the executable. This validates
component compilation only, not mounting on the router or complete image integration.

Repeat from the repository root:
`bash vendor/gt-be98-pro/open-source/build-ntfs.sh /absolute/empty-output-directory`.
The output includes the compile log, ELF report, image ID and SHA256SUMS. No vendor
build tree is modified. Full firmware generation still requires a populated build
checkout, verified remaining inputs and the local toolchain kit.

## Queue core implementation

`gso/ring.h` is a source implementation of bounded descriptor storage and batch
extraction. `gso/test_ring.c` passes under GCC with AddressSanitizer and UBSan in
the build container. It exercises 8,193 full/drain cycles (multiple 16-bit wraps),
FIFO ordering, overflow, empty/partial batches and descriptor preservation.
The API requires external serialization; these tests do not test kernel races.
It is not linked into firmware yet and does not implement a complete GSO module.

Worker interface observations: baseline sw_gso_task at 0xaf4 selects a batch limit
of 16; 0xafc–0xb68 copies 32-byte job records while holding the queue lock and
advances the 16-bit consumer; 0xbc0–0xbcc calls bcm_sw_gso_skb_prehandle with the
packet, device and two callbacks in the non-assist path. The assist path for larger
batches has additional shared queues and callback substitution and remains pending.
The ring core does not claim to implement that path, shutdown or flow reassignment.

Re-run the C checks from the repository root:

```sh
docker run --rm --user "$(id -u):$(id -g)" \
  -v "$PWD/vendor/gt-be98-pro/open-source/gso:/source:ro" amng-build \
  bash -c 'gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined /source/test_ring.c -o /tmp/test-ring && /tmp/test-ring'
```

Kernel configuration validation now executes the actual extraKernelConfig writer
against a temporary configuration. Its inputs must omit the CONFIG_ prefix,
because the writer adds it. The test confirms real output entries and catches
doubled prefixes, not just Make-variable values. A complete kernel build and
final-image module inventory are still required.

## Source GSO dispatch selection

The experimental profile now keeps BCM_SW_GSO=y and sets BCM_SW_GSO_THREAD=n.
The source at kernel/bcmkernel/net/core/bcm_sw_gso.c explicitly falls back to
synchronous segmentation when no worker callback is registered. This replaces
the proprietary scheduling layer with an existing source path; it does not remove
segmentation. Performance and CPU distribution may differ. Underlying Broadcom
buffer/accelerator dependencies remain; this is not fully independent networking.

Kconfig.bcm defines THREAD as a separate tristate dependent on SW_GSO. The
bcmdrivers Makefile selects sw_gso by that tristate, and
`targets/fs.src/rom/etc/init.d/bcm-base-drivers.list` gates its load entry on the
same symbol. Final-image absence and successful traffic still need verification.

`python3 vendor/gt-be98-pro/open-source/test-gso-fallback.py` extracts the actual
three dispatch functions from the current committed kernel source, compiles them
in Docker with mocked callbacks, and checks absent/busy-worker fallback and
packet ownership. AddressSanitizer/UBSan pass. This tests the real dispatch code,
not the packet segmentation algorithm or concurrency. Source version is HEAD;
uncommitted edits to those kernel functions are not part of this test.

The ring core remains available for a future threaded source implementation if
synchronous processing does not meet throughput requirements. No replacement
is considered hardware validated until its behavior and performance are measured.

## HFS utility portability

The first complete experimental build stopped while linking fsck_hfs: its two
strlcpy calls relied on a libc extension absent from the ARM target libc. A small
locally named bounded-copy function in diskdev_cmds' Linux compatibility header
supplies those calls without adding a binary library dependency. It returns the
source length and terminates the destination whenever its capacity is nonzero.

Validation: both HFS utilities were rebuilt from clean objects using the ARM
GCC 10.3 toolchain in Docker. Under qemu-arm, newfs_hfs formatted a disposable
64 MiB regular file and fsck_hfs -fn reported that volume healthy. Copy boundary
checks (zero capacity, one byte, truncation, exact fit, empty input) passed with
AddressSanitizer and UBSan. This is a basic utility smoke test; damaged-volume
repair, router mounting, and hardware storage tests remain outstanding. The full
firmware build must still succeed before this profile produces a usable artifact.

## Completed filesystem/GSO build

The experimental profile built successfully at commit `1b11141a5277` on
2026-11-11. The normal image is 76,658,404 bytes with SHA-256
`8546e8581f8042ad1e4e559026b060ccb20cdd26f94c271107a46995774bf528`.
FIT payload integrity checks passed. Inspection of its extracted SquashFS confirms
that tfat.ko, tntfs.ko, thfsplus.ko and sw_gso.ko are absent; fat.ko, vfat.ko,
hfs.ko, hfsplus.ko, ntfs-3g, mkfs.hfsplus and fsck.hfsplus are present. The rootfs
contains 151 kernel modules. This inventory does not prove all blobs are removed.
The image still includes boot components and other binary inputs; provenance is
partial and hardware behavior is untested. It is not published as an installable
release. The logging source added afterward was not part of this image.

## Logging replacement

The experimental profile now selects a source logging library. See
[logging/README.md](logging/README.md) for the test runner, scope and known
behavior differences.

## Current image validation

The profile built successfully at commit `99690c9fa5c6` on 2026-11-11. Its normal
image is 76,719,844 bytes with SHA-256
`87ad77477251560e18431533a8a77b6d4a0c8cd6f6a3d6113c9327c92e1b5d8a`.
FIT integrity checks passed. Extracted-image inspection confirms the source
logging library retains its required libshared dependency, the source FAT checker
and NTFS formatter are packaged, and the four replaced modules remain absent.

The packaged tools formatted disposable FAT and NTFS files under ARM emulation;
the FAT checker reported the new volume healthy. All 151 modules passed static
export/dependency/architecture checks against the matching packaged kernel.
The userspace scan covered 647 ELF objects and retained the same 81 standalone
or plugin-scope flags as the baseline, with no new or changed flags. Those flags
are not a claim that every standalone library can run independently. Static
checks do not validate private ABI layouts, dynamic loading, traffic or hardware.
Full NTFS consistency checking is unavailable. The image remains experimental,
with partial provenance and other proprietary components still present.

## NVRAM source components

[nvram/README.md](nvram/README.md) describes the protocol, transport, settings and
persistence implementation and its Docker test runner. These components are not
yet integrated as a replacement for the installed NVRAM libraries. Notification,
cache lifetime, ASUS storage hooks and full public API coverage remain required.
