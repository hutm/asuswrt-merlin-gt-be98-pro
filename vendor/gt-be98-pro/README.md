# GT-BE98 Pro downstream maintenance

Status: development only. No firmware is approved for publication or installation.
The current source baseline is Merlin 3006.102.9, commit
`69838e4a60564e5550e97f11d8a8cf2c6260c4f0`.

## Keep the fork small

Keep provenance and build orchestration in this directory. Preserve upstream
source layout and history. Do not delete other models or rename upstream APIs.
This fork supports only `gt-be98_pro`; retained upstream targets are unsupported.

Do not commit downloaded archives, extracted binaries, build output, or local
logs. `inputs.json` maps ASUS archive members to build inputs. Stage these only
in a disposable build checkout. Each archive needs an ASUS HTTPS URL and a
published SHA-256; each input needs an exact member hash. A match against a
Merlin repository blob, symbol table or version string is not vendor provenance.
Archive verification establishes equality to ASUS-published bytes, not absence
of vulnerabilities or compromise at ASUS.

Keep each compatibility fix in a separate commit documenting the old API,
replacement, source revision, security implications and validation. Scope changes
to this model/build where necessary. Never silently revert security fixes, add
no-op stubs for missing APIs, or modify proprietary inputs to make linking pass.

## Verify and stage

Download the URL recorded in `inputs.json` to a cache outside Git. The verifier
does not download or execute vendor content. Use Python 3.9 or later:

```sh
python3 vendor/gt-be98-pro/provenance.py stage --archive /path/to/GPL_GT_BE98_PRO_300610238984.zip
python3 vendor/gt-be98-pro/provenance.py verify
python3 vendor/gt-be98-pro/provenance.py release-check
python3 -m unittest discover -s vendor/gt-be98-pro -p 'test_*.py'
```

The manifest currently covers 280 userspace inputs, not the whole image. `verify`
only checks those entries. `release-check` deliberately fails regardless of the
JSON status until complete input/link tracking and final-image verification exist.
No release workflow or automatic firmware upload is enabled.

Compile only inside Docker as a non-root builder. The existing local build kit
is not yet a self-contained, portable part of this fork; documenting and importing
its reproducible setup remains a release blocker. Builds must distinguish vendor
objects from newly compiled open-source objects and record the actual link inputs,
including archive members. Exact final-file matching alone is insufficient.

## Rebase procedure

Maintain a small linear downstream series on a stable Merlin tag. Track upstream
main for upcoming changes, but do not automatically release development snapshots.
Work in a clean source checkout, never the checkout with staged blobs/build output.

Run `python3 vendor/gt-be98-pro/check-upstream.py` to resolve the latest stable
3006.102 tag and compare its commit to the manifest pin. It excludes prerelease
tags, sorts version numbers numerically, and makes no local changes. This command
does not establish authenticity of upstream Git tags or binary inputs.

```sh
git remote add upstream https://github.com/RMerl/asuswrt-merlin.ng.git # once
git fetch upstream --tags
git switch -c candidate/NEW_TAG gt-be98-pro/asus-verified
git rebase --onto NEW_TAG 69838e4a60564e5550e97f11d8a8cf2c6260c4f0
```

Use the currently recorded `merlin_base` as the old base on subsequent rebases.
Review conflicts one commit at a time. Update that pin only after the rebase;
invalidate all previous build and runtime evidence. Check for new or relocated
proprietary inputs and changed APIs, stage the verified archive into a fresh build
checkout, rebuild, and re-run the complete audit. A clean Git rebase is not proof
of binary compatibility. Preserve immutable release tags; publish candidates on
new branches rather than rewriting a released branch/tag.

Before firmware publication, require complete provenance, successful linking and
image construction, dependency/ABI checks, and hardware tests on GT-BE98 Pro:
boot/reboot, Ethernet ports and VLAN isolation, Wi-Fi bands, DHCP/DNS, IPv6,
VPN, acceleration and recovery. Record which tests actually ran and their results.

## Updates

Watch ASUS's GT-BE98 Pro firmware and GPL catalogs separately, and the ASUS
security feed: https://security-advisory-api.asus.com/rss/securityrss.xml .
Retain published archive checksums and inventory changes. A new stock security fix
does not prove an older GPL blob is patched. Track this as an unresolved update
until the affected component and fix are established. No scheduled monitor is
configured by this initial fork.

ASUS 102_39442 is a verified runtime reference, not the current build-input base.
Its published ZIP checksum is
`eb79b00ba6a68a062dda48c8e0c7bb81041819661b4913ae861d591bab05fa72`.
Do not describe a build as 39442-based merely because it was compared with it.
