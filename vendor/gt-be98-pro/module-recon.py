#!/usr/bin/env python3
"""Read-only kernel module reconnaissance; requires pyelftools==0.32.

License strings and same-named source files are evidence, not a completeness
proof. Never executes firmware or claims decompilation establishes correctness.
"""
import argparse
import collections
import hashlib
import io
import json
import os
from pathlib import Path
import subprocess

from elftools.elf.elffile import ELFFile


def inspect(path):
    data = path.read_bytes()
    elf = ELFFile(io.BytesIO(data))
    info = collections.defaultdict(list)
    section = elf.get_section_by_name('.modinfo')
    if section:
        for item in section.data().split(b'\0'):
            if b'=' in item:
                key, value = item.decode(errors='replace').split('=', 1)
                info[key].append(value)
    table = elf.get_section_by_name('.symtab')
    functions, imports = [], []
    if table:
        for symbol in table.iter_symbols():
            if not symbol.name:
                continue
            if symbol['st_shndx'] == 'SHN_UNDEF':
                imports.append(symbol.name)
            elif symbol['st_info']['type'] == 'STT_FUNC':
                functions.append({'name': symbol.name, 'size': symbol['st_size'],
                                  'binding': symbol['st_info']['bind']})
    return dict(sha256=hashlib.sha256(data).hexdigest(), bytes=len(data),
                executable_section_bytes=sum(s['sh_size'] for s in elf.iter_sections()
                                             if s['sh_flags'] & 4),
                architecture=elf['e_machine'], modinfo=dict(info),
                named_function_symbols=len(functions),
                functions=sorted(functions, key=lambda f: (-f['size'], f['name'])),
                undefined_symbols=sorted(set(imports)),
                debug_info_present=elf.get_section_by_name('.debug_info') is not None)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rootfs', required=True, type=Path)
    parser.add_argument('--repo', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    files = subprocess.check_output(['git', '-C', str(args.repo), 'ls-files', '-z'], text=True).split('\0')
    by_stem = collections.defaultdict(list)
    for name in files:
        p = Path(name)
        if name.startswith('release/src-rt-5.04behnd.4916/') and p.suffix in ('.c', '.S'):
            by_stem[p.stem].append(name)
    modules, errors = [], []
    for directory, _, names in os.walk(args.rootfs, followlinks=False,
                                      onerror=lambda error: errors.append(str(error))):
        for name in sorted(names):
            path = Path(directory) / name
            if not name.endswith('.ko') or path.is_symlink():
                continue
            row = inspect(path)
            row['path'] = str(path.relative_to(args.rootfs))
            licenses = row['modinfo'].get('license', [])
            row['explicit_nonfree_license'] = any('proprietary' in s.lower() or
                                                  'commercial' in s.lower() for s in licenses)
            row['same_named_source_candidates'] = by_stem[path.stem]
            row['source_completeness'] = 'not-established-by-this-scan'
            modules.append(row)
    modules.sort(key=lambda r: r['path'])
    report = dict(scope='Kernel .ko files in supplied extracted rootfs; excludes host tools and userspace blobs.',
                  source_completeness_warning='Source candidates and module licenses do not prove all linked code has source.',
                  module_count=len(modules),
                  explicit_nonfree_count=sum(r['explicit_nonfree_license'] for r in modules),
                  read_errors=errors, modules=modules)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k:v for k,v in report.items() if k != 'modules'}, indent=2))
    for row in modules:
        if row['explicit_nonfree_license']:
            print(f"{row['path']}: {row['named_function_symbols']} named functions; "
                  f"{row['executable_section_bytes']} executable bytes; "
                  f"{len(row['same_named_source_candidates'])} same-named source candidates")


if __name__ == '__main__':
    main()
