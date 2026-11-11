# Reducing proprietary runtime code

This fork does not currently promise a fully open-source firmware. Its immediate
goal is traceable ASUS-origin proprietary inputs alongside source-built open-source
components. Reimplementing all proprietary components is a separate development
project and would conflict with keeping the downstream patch set small.

## What the binary inventory means

An ELF file, `.o`, `.a`, or `.ko` is not necessarily proprietary. Some prebuilt
files have corresponding open source. Some modules combine source-built glue and
closed objects. Multiple files can belong to one subsystem. Classify actual build
inputs and their licenses/source availability before calling them closed source;
the current 280-entry manifest is a provenance inventory, not a closed-component
count. Exclude host distribution tools from the runtime scope.

## Feasibility assessment (not validated replacement support)

| Area | Direction | Main limitation |
|---|---|---|
| Linux core, standard modules, network services | Build available source | Confirm complete corresponding source and actual link inputs |
| Optional ASUS cloud/gaming services and proprietary traffic inspection | Remove complete features where dependencies permit | UI, service startup and dependent code must agree; removal alone is not tested |
| Proprietary filesystem implementations | Evaluate existing open-source filesystem drivers or omit optional formats | Kernel version, integration and USB storage compatibility |
| ASUS private objects in shared libraries, rc and httpd | Replace small, documented interfaces individually | NVRAM formats, configuration migration, authentication and IPC contracts |
| Broadcom Ethernet/acceleration stack | Investigate source drivers and software forwarding | Board/SoC support and throughput cannot be assumed; disabling acceleration may not remove driver dependencies |
| Broadcom Wi-Fi kernel objects and radio firmware | Hardware-specific driver and firmware work | No verified open replacement for this router's full Wi-Fi 7 stack identified |

Linux's brcm80211 documentation lists supported chips and distinguishes host
drivers from device firmware. It does not establish GT-BE98 Pro support:
https://wireless.docs.kernel.org/en/latest/en/users/drivers/brcm80211.html

OpenWrt explicitly documents limited Broadcom support because of limited free
driver availability:
https://openwrt.org/meta/infobox/broadcom_wifi

These sources are evidence of constraints, not proof that every possible upstream
or experimental driver has been exhaustively searched. OpenWrt support on some
other device also does not imply that its radio firmware is open source.

## Work order

1. Complete the deployed-input inventory, including proprietary archive members
   embedded in otherwise source-built executables and libraries.
2. Rebuild source-available components before attempting interface reconstruction.
3. Remove optional closed features in a separately tested minimal profile.
4. For a small remaining interface, document behavior and implement an independent
   replacement with positive, failure-path, and compatibility tests. Generated
   output alone is not a correct or maintainable source implementation.
5. Treat wireless, switch and packet-processing replacement as dedicated hardware
   bring-up work requiring recoverable test hardware, diagnostics and sustained
   testing. Full radio firmware replacement also requires understanding calibration,
   timing, DMA and hardware control; making a host module compile is insufficient.

Do not replace working vendor implementations with no-op stubs or claim an image
is open source because closed code was disassembled into recompilable assembly.
No fully open-source milestone or completion date is promised for this hardware.
