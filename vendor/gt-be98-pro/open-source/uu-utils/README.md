# Model eligibility helper

The source candidate supplies the single `uu_model_check` export. It calls
`get_model` followed by `get_productid`, once each, and examines a product-name
snapshot truncated to 31 bytes. Matching is case-sensitive. It uses the existing
named model constants so the mapping remains readable when rebasing.

A product snapshot containing `ExpertWiFi` returns zero. The remaining rules are:

| Model | Result |
| --- | --- |
| RTAC82U | One only for `RT-AC2200` |
| RTAC68U | Zero only for `RP-AC1900` |
| RTAX58U | Zero only for `RT-AX58U` |
| RTAX82U_V2 | One only for `RT-AX82U` |
| TUFAX5400_V2 | One only for `TUF-AX5400` |
| RTAX5400 | One only for `RT-AX5400` |
| RTAX82_XD6S, XD6_V2 | One only for `ZenWiFi_XD6` |
| EBA63, BM68, BR63, EBG19, EBG15 | Zero |
| All other model IDs | One |

A null product pointer is treated as an empty string. This produces the same
return values as the reference's libc null-string formatting for these rules.
The helper does not modify settings or invoke a service. It preserves the
existing policy; it does not replace the associated plugin or service binaries.

The suite checks the export and compares 48442 cases against the original ARM
object and native ASan/UBSan execution. It covers model IDs -2 through 512 plus
INT_MIN/INT_MAX, named products, whitespace changes, case changes, null/empty
names and substring positions around the truncation boundary. Both provider
calls and their ordering are checked. Dynamic-symbol inspection of the previous
image found consumers in rc and libwebapi; that does not prove complete coverage
of dynamic lookup or internal static calls.

```sh
ASUS_REFERENCE_ROOT=/absolute/extracted/reference/root \
bash vendor/gt-be98-pro/open-source/uu-utils/test.sh
```

Full-image and hardware validation remain pending.
