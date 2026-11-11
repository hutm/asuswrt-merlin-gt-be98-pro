#!/usr/bin/env python3
"""Verify/stage declared ASUS inputs. Partial inventory never authorizes release."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import sys
import tarfile
import tempfile
import zipfile

HERE = Path(__file__).resolve().parent


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def contained(root, name):
    p = PurePosixPath(name)
    if p.is_absolute() or '..' in p.parts or not p.parts:
        raise ValueError('Unsafe manifest path: ' + name)
    result = root / name
    if not result.resolve().is_relative_to(root.resolve()):
        raise ValueError('Path escapes checkout: ' + name)
    if result.is_symlink():
        raise ValueError('Refusing symlink destination: ' + name)
    return result


def verify(root, manifest):
    errors = []
    for row in manifest['inputs']:
        p = contained(root, row['destination'])
        if not p.is_file() or digest(p) != row['sha256']:
            errors.append(row['destination'])
    if errors:
        raise ValueError('Missing/mismatched declared inputs:\n' + '\n'.join(errors))
    print(f"Verified {len(manifest['inputs'])} declared inputs; coverage={manifest['coverage']}")


def stage(root, manifest, archive):
    if digest(archive) != manifest['archive']['sha256']:
        raise ValueError('ASUS archive checksum mismatch; nothing staged')
    expected = {}
    destinations = set()
    for row in manifest['inputs']:
        contained(root, row['destination'])
        if row['destination'] in destinations:
            raise ValueError('Duplicate destination')
        destinations.add(row['destination'])
        expected.setdefault(row['archive_member'], []).append(row)
    # Validate every requested member before changing any checkout files.
    with tempfile.TemporaryDirectory(prefix='asus-inputs-') as tmp:
        cache = Path(tmp)
        found = set()
        with zipfile.ZipFile(archive) as z:
            names = [n for n in z.namelist() if n.endswith('.tgz')]
            if len(names) != 1:
                raise ValueError('Expected exactly one tgz in ASUS archive')
            with z.open(names[0]) as stream, tarfile.open(fileobj=stream, mode='r|gz') as tar:
                for member in tar:
                    if member.name not in expected:
                        continue
                    if member.name in found or not member.isfile():
                        raise ValueError('Duplicate or nonregular archive member')
                    data = tar.extractfile(member).read()
                    sha = hashlib.sha256(data).hexdigest()
                    if any(row['sha256'] != sha for row in expected[member.name]):
                        raise ValueError('Archive member checksum mismatch: ' + member.name)
                    (cache / sha).write_bytes(data)
                    found.add(member.name)
        if found != expected.keys():
            raise ValueError('Missing archive members: ' + repr(expected.keys() - found))
        for rows in expected.values():
            for row in rows:
                target = contained(root, row['destination'])
                if not target.is_file():
                    raise ValueError('Upstream destination disappeared: ' + str(target))
        for rows in expected.values():
            for row in rows:
                target = contained(root, row['destination'])
                target.write_bytes((cache / row['sha256']).read_bytes())
    verify(root, manifest)


def release_gate(manifest, status):
    # Intentionally fail closed until a complete input/link and image audit is
    # implemented. Changing a JSON boolean cannot enable firmware publication.
    raise ValueError('Firmware publication disabled: complete audit not implemented.\n'
                     + '\n'.join(status['blockers']))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['verify', 'stage', 'release-check'])
    parser.add_argument('--model', default='gt-be98_pro', choices=['gt-be98_pro'])
    parser.add_argument('--root', type=Path, default=HERE.parents[1])
    parser.add_argument('--archive', type=Path)
    args = parser.parse_args()
    manifest = json.loads((HERE / 'inputs.json').read_text())
    try:
        if args.command == 'stage':
            if args.archive is None:
                parser.error('stage requires --archive')
            stage(args.root, manifest, args.archive)
        elif args.command == 'verify':
            verify(args.root, manifest)
        else:
            release_gate(manifest, json.loads((HERE / 'release-status.json').read_text()))
    except (ValueError, OSError, tarfile.TarError, zipfile.BadZipFile) as exc:
        print(str(exc), file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
