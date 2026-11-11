# Public ASUS GPL compatibility profile

Opt in with environment variable `ASUS_PUBLIC_GPL_38984=y`. The rc/shared
Makefiles reject this profile for other models. Without it upstream behavior is
unchanged. Keep this profile separate from generic upstream/security changes.

## Inputs

The base remains GT-BE98 Pro GPL 102_38984. Three userspace inputs, `spwenc.o`, `web_hook.o`, and `libbwdpi_sql.so`,
come from the publicly downloadable GT-BE98 GPL 102_39274, recorded separately
in `supplemental-inputs.json`. Its archive and member hashes are verified before
staging. No GT-BE98 radio, switch driver or board initialization object is imported.
The ARM `spwenc.o` object supplies `safe_do_system` and `init_asus_pp_eula`;
their existing source call sites and input-validation behavior are retained.
`web_hook.o` supplies the NVRAM change logging APIs used by current `web.c`;
logging is retained instead of removing these calls. Its model-specific UI
behavior, including the Pro radio layout, still needs hardware validation.
`libbwdpi_sql.so` supplies the traffic-monitoring CGI APIs reached by the newer
web hooks. Its database compatibility with the older runtime also needs testing.
Cross-model integration still requires runtime validation; provenance alone does
not prove API/behavioral compatibility.

## Source differences

* Disable `MULTISERVICE_WAN` for GT-BE98 Pro in this profile. The ASUS 38984
  `release/src-rt/target.mak` does not enable it for this model and its archive
  supplies no `mswan-broadcom.o`. This also avoids retaining an unverified generic
  object from a newer Merlin base. Multi-service WAN is an unsupported feature
  of this profile, not a successfully implemented no-op. Ordinary WAN, dual WAN,
  and existing IPTV settings are not deliberately disabled; they require tests.
* Omit the `wl_scb` applet and its installation symlink, `ed_thresh_clear` shutdown
  call, and `backup_eth_ob_log` call. These calls are absent in the verified
  ASUS 38984 rc source and were added to Merlin by commit
  `95fe368c0e` (GPL 102_39848 merge). Older proprietary objects do not supply them.
  The older shutdown and onboarding paths are retained, without suppressing
  authentication or input-validation checks.
* Implement `is_mxl_dual_serdes_war()` for this model as false. This is a hardware
  capability result, not a placeholder for an unknown implementation: the
  GT-BE98 Pro ASUS 102_39442 `usr/lib/libshared.so` disassembly ends all successful
  paths with `mov r0, #0`. It reads an NVRAM value but does not use it to change the
  result. The profile rejects compilation on a MaxLinear switch configuration.
  Reference archive SHA-256:
  `eb79b00ba6a68a062dda48c8e0c7bb81041819661b4913ae861d591bab05fa72`.

Build success, complete input provenance, final-image API validation and hardware
testing are separate milestones. Consult `release-status.json` before publishing.
On a future rebase, revisit each guard and remove this compatibility profile when
a verified Pro package supplies the required implementations. Do not expand this
file into a general emulation layer for proprietary APIs.
