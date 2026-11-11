#!/usr/bin/env python3
"""Trace existing Kbuild .cmd files without executing their contents.

This is reconnaissance, not a complete dependency/license proof. It assumes the
BCM4916 kernel build's working directory and records unrecognized/missing inputs.
"""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import shlex
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    repo = args.repo.resolve()
    platform = repo / 'release/src-rt-5.04behnd.4916'
    kernel = platform / 'kernel/linux-4.19'
    commands = subprocess.check_output(['rg', '--files', '--hidden', '-uuu', '-g',
                                       '*.ko.cmd', str(platform)], text=True).splitlines()
    by_name = collections.defaultdict(list)
    for command in commands:
        p = Path(command)
        by_name[p.name[1:-4]].append(p)
    objects = {}

    def resolve(name):
        p = Path(name)
        if not p.is_absolute():
            p = kernel / p
        for _ in range(8):
            if str(p).startswith('/asuswrt/'):
                p = repo / str(p)[len('/asuswrt/'):]
            resolved = p.resolve()
            if resolved == p:
                return p
            p = resolved
        raise ValueError('Cannot map build path: ' + name)

    def label(p):
        return str(p.relative_to(repo)) if p.is_relative_to(repo) else str(p)

    def trace(path):
        path = resolve(str(path))
        key = label(path)
        if key in objects:
            return key
        row = objects[key] = {'path': key}
        command = path.with_name('.' + path.name + '.cmd')
        if not command.exists():
            row.update(evidence='no-kbuild-command', exists=path.exists())
            if path.is_file():
                row['sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
            return key
        first = command.read_text(errors='replace').splitlines()[0]
        row['command_file'] = label(command)
        if ':=' not in first:
            row['evidence'] = 'unrecognized-command'
            return key
        tokens = shlex.split(first.split(':=', 1)[1])
        sources = [resolve(s) for s in tokens if s.endswith(('.c', '.S'))]
        if sources:
            row.update(evidence='compiled-source', sources=[
                dict(path=label(p), exists=p.is_file(), module_metadata=p.name.endswith('.mod.c'))
                for p in sources])
            return key
        deps = []
        for i, token in enumerate(tokens):
            if token.endswith(('.o', '.a')) and (i == 0 or tokens[i-1] != '-o'):
                dep = resolve(token)
                if dep != path:
                    deps.append(trace(dep))
        row.update(evidence='linked-objects' if deps else 'unresolved-command', inputs=deps)
        return key

    inventory = json.loads(args.inventory.read_text())
    modules = []
    for module in inventory['modules']:
        name = Path(module['path']).name
        modules.append(dict(module=module['path'], kbuild_trees=[
            trace(p.with_name(name)) for p in by_name.get(name, [])]))
    report = dict(scope='Existing Kbuild commands; source compilation does not prove open licensing or complete provenance.',
                  modules=modules, objects=objects)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(dict(modules=len(modules),
                          modules_with_commands=sum(bool(m['kbuild_trees']) for m in modules),
                          object_evidence=dict(collections.Counter(r['evidence'] for r in objects.values()))), indent=2))


if __name__ == '__main__':
    main()
