# Local iteration and hardware test procedure

This is an experimental GT-BE98 Pro build. No script here flashes a router.
The local Docker/toolchain kit is still required; a fresh clone alone is not
sufficient. Automated checks cannot guarantee recovery or absence of malware.

## Three commands

From the fork checkout, with the existing `../asuswrt-build` kit:

```sh
python3 vendor/gt-be98-pro/iterate.py check
python3 vendor/gt-be98-pro/iterate.py build
python3 vendor/gt-be98-pro/iterate.py status
```

`check` verifies both pinned input manifests, upstream ancestry, Docker access,
and build prerequisites. `build` repeats preflight, serializes iterations per
checkout, saves a launcher snapshot and separate log, builds inside Docker as the
non-root builder, verifies declared inputs again, and archives one newly written
normal image with SHA256SUMS and a JSON result. It validates SHA256 payload hashes
in the FIT image and nested FIT payloads, saving their component inventories. Failures produce a failed result
and nonzero exit; an old image cannot be reported as a new successful build.
Use `--kit /path/to/asuswrt-build` or `--runs /path/to/evidence` if needed.

Each run lives under `../asuswrt-build/iterations/<UTC-time>-<unique-id>/`.
Loader variants are not copied to iteration artifacts. The normal image still
contains ATF/U-Boot inside bootfs: its filename does not establish which flash
partitions the updater writes. Image hashes detect later changes; they are not
vendor signatures. The current automated workflow does not repeat the historical
ELF or kernel-symbol checks; those must be rerun and tied to the exact new
image before hardware testing. Prior build-result.json is historical evidence,
not validation of a new iteration.

Keep changes small, commit intended source changes, and use this runner rather
than invoking the old local launcher directly. Do not edit inputs, scripts, or
sources during a build. The launcher is snapshotted, but its source tree and kit
are not isolated snapshots. The tracked-change list is diagnostic evidence, not
a complete record of all untracked or generated build inputs. Interrupted runs
may retain a `building` result: inspect the log and Docker before restarting.
Do not use `git add .` in the populated build tree. For rebases use a separate
clean checkout and restage verified inputs. Never run destructive cleanup on the
only copy of local artifacts. See README.md for the rebase process.

## Before the first hardware test

Keep the primary router on known-good firmware. Use a spare **GT-BE98 Pro** with
stable power, a wired test PC and an isolated network. Record the exact model and
hardware revision. No bootloader, flash layout, calibration or board-data changes
are acceptable without a separate review and a device-specific recovery plan.

Prepare the recovery PC and official ASUS firmware offline before testing.
The already verified reference download is:

- URL: https://dlcdnets.asus.com/pub/ASUS/wireless/GT-BE98_Pro/FW_GT-BE98_Pro_300610239442.zip
- ZIP SHA256: `eb79b00ba6a68a062dda48c8e0c7bb81041819661b4913ae861d591bab05fa72`

This is a pinned recovery reference, not a claim that it is the newest or safest
ASUS release. Verify the ZIP against its published checksum, extract it and retain
the exact normal firmware filename/hash locally. Back up configuration privately;
backups may contain credentials. Do not publish logs with secrets or restore old
configuration blindly into a different firmware version.

ASUS documents recovery from failed upgrades and third-party firmware problems
using its Windows Firmware Restoration utility:
https://www.asus.com/support/faq/1000814/ . Follow the exact model's manual:
https://dlcdnets.asus.com/pub/ASUS/wireless/GT-BE98_Pro/E23237_GT-BE98_PRO_UM_WEB.pdf?model=GT-BE98+Pro
(section 4.2). Prove rescue-mode entry and PC connectivity on the spare first.
Record the actual recovery address/adapter setup; generic examples may differ
from the model manual. A factory reset only clears settings; it is not reflashing.
Rescue mode depends on working boot/recovery code, so recovery is not guaranteed.
Damage to bootloader or device-specific flash data can require service or external
programming. Never interrupt an ongoing flash operation.

Before any candidate upload, require these reviewed evidence items:

1. Exact image SHA256; fresh build and FIT payload checks; correct model/layout.
2. Complete provenance review for included proprietary inputs, and no unexplained
   dependency/symbol failures. Cross-model private APIs still require runtime tests.
3. Review of the updater's partition writes, including bootfs boot stages. Do not
   flash `_loader.pkgtb` as part of the iteration workflow.
4. Successful recovery rehearsal and a ready stock image, wired PC and stable power.

The publication gate remains closed: `iterate.py release-check` deliberately
returns nonzero. A successful compile cannot override these outstanding items.

## Hardware evidence, one candidate at a time

Record the candidate hash, hardware revision, test date, configuration and result
for each test. Untested means untested, never passed. Start isolated and wired:
boot and reboot, administration access, DHCP reservations and DNS, all Ethernet
ports, link negotiation, and VLAN/guest isolation. Then test Wi-Fi bands, client
reconnects, IPv6, VPN, Route53 update behavior, and throughput/acceleration under
load. Test fail/recovery behavior only with an agreed procedure; do not simulate
power loss during flashing. If a test fails, preserve logs, return to known-good
stock via the documented method if needed, and change one thing before rebuilding.
Do not publish a firmware release or move to the primary router until the evidence
is reviewed. Keep private test observations outside the public Git repository.
