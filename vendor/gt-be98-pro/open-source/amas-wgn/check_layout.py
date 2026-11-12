#!/usr/bin/env python3
"""Generate ABI probes from Merlin's public record declarations and local types."""
import pathlib
import re
import sys
public = pathlib.Path(sys.argv[1]).read_text()
out = pathlib.Path(sys.argv[2])
types = {
    'wgn_subnet_rule': 'ipaddr ipmask dhcp_enable dhcp_start dhcp_end dhcp_lease domain_name dns wins ema macipbinding type'.split(),
    'wgn_vlan_rule': 'enable vid prio wanportset lanportset wl2gset wl5gset subnet_name internet public_vlan type'.split(),
}
declarations = []
for name in types:
    match = re.search(r'typedef struct ' + name + r'_t\s*\{.*?\}\s*' + name + r';', public, re.S)
    if not match:
        raise SystemExit('Public record declaration missing: ' + name)
    declarations.append(match.group())
body = ['int main(void) {']
for name, fields in types.items():
    body.append(f'printf("{name}:%zu\\n", sizeof({name}));')
    for field in fields:
        body.append(f'printf("{name}.{field}:%zu:%zu\\n", offsetof({name}, {field}), sizeof((({name} *)0)->{field}));')
body.append('return 0; }')
base = '#include <stdio.h>\n#include <stddef.h>\n'
(out / 'public-layout.c').write_text(base + '\n'.join(declarations + body))
(out / 'candidate-layout.c').write_text(base + '#include "subnet.h"\n#include "vlan.h"\n' + '\n'.join(body))
