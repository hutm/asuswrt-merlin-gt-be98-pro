# Kernel module source coverage assessment

This is an exploratory assessment, not a replacement-driver release. The initial
scan uses the previously built standard Merlin 3006.102.9 rootfs, not a claim
about a completed ASUS-input integration image. No firmware code was executed
during the scan.

## Coverage and evidence

* Inspected all 153 `.ko` files in the baseline module inventory, collecting
  hashes, license metadata, dependencies, imports, named function symbols, code
  sizes, and candidate source filenames. Six unreadable certificate directories
  were recorded by the broad filesystem walk; the discovered module path set
  matches the existing 153-module ELF inventory.
* 17 modules explicitly identify themselves as proprietary/commercial. Generated
  local AArch64 disassembly for all 17 (about 128 MB of text). This is preparation
  for behavioral analysis, not 17 completed reimplementations.
* Another 17 modules match prebuilt inputs despite GPL/dual-license declarations.
  These include `bcm_enet`, `dhd`, `hnd`, `bdmf`, `wfd`, and the Realtek switch
  module. Source completeness must be traced; a GPL module label is insufficient.
* Located Kbuild command evidence for 147 modules and followed 710 object nodes:
  458 had source-compilation commands, 203 linked other objects, 48 lacked command
  evidence, and one command remained unresolved. The six modules without Kbuild
  roots were Tuxera filesystem and Trend Micro modules. Missing command records
  are not automatically proof of closed source; some referenced paths were no
  longer available on the host, and generated code/license checks remain.

`module-recon.py` makes the first scan repeatable with Python and pyelftools 0.32:

```sh
python3 vendor/gt-be98-pro/module-recon.py \
  --rootfs /path/to/extracted/rootfs --repo . --output /path/to/module-report.json
```

Full disassembly and generated reports remain outside the source repository.
Do not commit hundreds of megabytes of derived binary listings into this fork.

## Explicitly proprietary/commercial modules

These classifications describe likely work direction, not validated removal.

| Modules | Work direction |
|---|---|
| `tfat`, `tntfs` | Prefer existing open-source filesystem implementations or drop optional filesystem support |
| `tdts`, `tdts_udb`, `tdts_udbfw` | Prefer removing the complete proprietary inspection feature and its dependent UI/services |
| `sw_gso` | Bounded candidate for an independently tested replacement |
| `bcm_bp3drv`, `bcm_bpm`, `bcm_mpm`, `bcm_pondrv`, `bcmvlan`, `cmdlist`, `gdx`, `pktflow`, `pktrunner`, `rdpa` | Trace hardware interfaces and dependency graph; investigate source drivers/software forwarding before reimplementation |
| `wl` | Large wireless-driver project, coupled to radio firmware and vendor interfaces |

Scale from the baseline symbol tables: `sw_gso` has 18 named function symbols and
6,518 executable bytes; `rdpa` has 4,895 symbols and 1,523,246 bytes; `wl` has
10,470 symbols and 6,688,310 bytes. Symbols include aliases and any source-built
portions; these are not counts of independent closed-source functions.

## Initial bounded pilot: sw_gso

The build recipe explicitly links `sw_gso_hook.o` (source supplied) and
`sw_gso_bin.o` (prebuilt when `sw_gso_bin.c` is absent). The open wrapper and
`sw_gso.h` expose buffer/queue, locking, callback and wait-queue interfaces.
Eight named wrapper functions already have source. The other ten symbol names
include init/cleanup aliases; they must not be counted as ten unique algorithms.

Initial disassembly findings:

* `sw_gso_enqueue` receives the packet, net device and two callbacks, writes four
  pointers into a ring entry, uses a bottom-half spinlock, publishes with a store
  barrier, and wakes a worker through an atomic state transition.
* The ring index is masked with capacity minus one, suggesting a power-of-two
  capacity requirement. Initialization and overflow behavior still need full
  reconstruction and tests.
* `sw_gso_task` is a kernel-thread worker that drains jobs in batches. Scheduling,
  rehashing, shutdown and buffer ownership remain to be fully specified.
* `sw_gso_log_init` reads the supplied logging structure's context pointer and
  stores it in `__check_fun`. The available `bcmLogModuleInfo_t` layout and
  `bcmLog_logIsEnabled` source help interpret this; guessing from the function
  name alone would be insufficient.

A useful next milestone is a complete behavioral specification and a host-testable
queue model, followed by a kernel implementation in a separate experimental branch.
Tests must cover queue overflow, wraparound, callback failure, shutdown races,
packet ownership and ordering. Only then should it be exercised on recoverable
GT-BE98 Pro hardware. No replacement module has yet been written or loaded.

Strong code models can accelerate this work, but neither generated C nor a
successful kernel-module link establishes equivalent behavior. Radio firmware
replacement remains a separate problem even if every host kernel module were
source-buildable. Keep experimental reconstruction out of the minimal ASUS-input
compatibility branch until it has independent evidence.
