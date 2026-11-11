# GT-BE98 Pro: ASUS-verified inputs experiment

Experimental fork of Asuswrt-Merlin for **ASUS GT-BE98 Pro only** (not GT-BE98).
This is an independent project, not an ASUS or upstream Merlin release.

**No installable firmware release is available.** An experimental firmware image
now builds and passes static image/dependency checks. Input provenance is still
incomplete, cross-model userspace compatibility is untested, and there has been
no hardware test. See [build results](vendor/gt-be98-pro/build-result.json).
The word “verified” describes individual inputs checked against an ASUS-published
archive checksum; it is not a claim that the current tree or an image is verified.

Our target is open-source components built from source plus unchanged proprietary
inputs from publicly downloadable ASUS archives with published checksums. ASUS
objects from multiple releases are allowed when compatibility is established.
We preserve upstream security fixes rather than suppressing missing APIs with stubs.

See [the build/provenance policy](vendor/gt-be98-pro/README.md),
[input manifest](vendor/gt-be98-pro/inputs.json), and
[release blockers](vendor/gt-be98-pro/release-status.json).
Upstream retains other models in its source tree; this fork's entry point accepts
only `gt-be98_pro`. Other models are unsupported.

Report fork-specific problems to this repository, not upstream Merlin support.

---

asuswrt-merlin New Gen (version 382.xx and higher)
==================================================

#### Support is available via the forums at [SNBForums](https://www.snbforums.com/forums/asuswrt-merlin.42/).

Asuswrt-Merlin is an enhanced version of Asuswrt, the firmware used by Asus's modern routers.

The goal of this project is to fix issues and bring some minor functionality adjustments to the 
original Asus firmware.  While some features do get added, this is not the main focus of this project.  
It is not meant to replace existing projects such as Tomato or DD-WRT, but rather to offer an alternative 
for people who prefer the original firmware featureset.

This is the new development branch, originally based on Asus's 
3.0.0.4.382_xxxx firmware release.  Development of the 380.xx 
legacy branch has been dropped.

Please consult the Wiki for an up-to-date list of supported models:

https://github.com/RMerl/asuswrt-merlin.ng/wiki/Supported-Devices
