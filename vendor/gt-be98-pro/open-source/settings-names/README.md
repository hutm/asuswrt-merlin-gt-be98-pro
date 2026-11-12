# Manufacturing settings-name helpers

The experimental GT-BE98_PRO profile selects this source object in place of
`shared/prebuild/shutils_private.o`. It supplies all three original exports:

* `ATE_FACTORY_MODE_STR` returns the same writable static eight-byte buffer on
  each call, resetting it to `ATEMODE` first. Callers must not free it. The legacy
  writable shared-buffer interface is retained; concurrent writes are not made
  thread-safe by this replacement.
* `ATE_UPGRADE_MODE_STR` returns a fresh allocated copy of `ateUpgrade_flag`.
  Callers own the allocation; allocation failure returns NULL.
* `ate_brcm_factory_mode` performs one `nvram_get` and returns one only for the
  exact string `1`. Missing, empty or other values return zero.

The component never changes settings or enters manufacturing mode. Existing
callers remain responsible for settings writes and freeing allocated strings.
Inspection of the earlier successful image found the name functions imported
by rc and httpd, with all three functions exported by libshared. Symbol-name
inspection does not cover dynamic lookup or prove complete caller coverage.

The comparison suite checks 774 settings values/cases, static-buffer reset and
identity, separate writable allocations and injected allocation failure. It
compares the original ARM object against the ARM source candidate and native
ASan/UBSan execution in non-root Docker. Settings reads are mocked; no physical
storage or hardware is accessed.

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/settings-names/test.sh
```

Full-image and hardware validation of this selection remain pending.
