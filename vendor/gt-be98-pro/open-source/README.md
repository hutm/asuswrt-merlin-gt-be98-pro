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
Local disassembly is outside Git under vendor-rebuild-39442/recon.

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
