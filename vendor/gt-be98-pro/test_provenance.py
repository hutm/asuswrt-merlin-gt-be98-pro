"""Exercise tampering and publication failures, using tiny synthetic archives."""
import hashlib
import io
import json
from pathlib import Path
import tarfile
import tempfile
import unittest
import zipfile

import provenance


class ProvenanceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.archive = self.root / 'vendor.zip'
        self.dest = self.root / 'object.o'
        self.dest.write_bytes(b'original')
        data = b'vendor bytes'
        stream = io.BytesIO()
        with tarfile.open(fileobj=stream, mode='w:gz') as tar:
            member = tarfile.TarInfo('asuswrt/object.o')
            member.size = len(data)
            tar.addfile(member, io.BytesIO(data))
        with zipfile.ZipFile(self.archive, 'w') as z:
            z.writestr('source.tgz', stream.getvalue())
        self.manifest = {
            'coverage': 'partial',
            'archive': {'sha256': provenance.digest(self.archive)},
            'inputs': [{'destination': 'object.o',
                        'archive_member': 'asuswrt/object.o',
                        'sha256': hashlib.sha256(data).hexdigest()}],
        }

    def test_stage_and_detect_later_tampering(self):
        provenance.stage(self.root, self.manifest, self.archive)
        self.assertEqual(self.dest.read_bytes(), b'vendor bytes')
        self.dest.write_bytes(b'tampered')
        with self.assertRaises(ValueError):
            provenance.verify(self.root, self.manifest)

    def test_archive_tampering_leaves_checkout_untouched(self):
        with self.archive.open('ab') as f:
            f.write(b'tampered')
        with self.assertRaises(ValueError):
            provenance.stage(self.root, self.manifest, self.archive)
        self.assertEqual(self.dest.read_bytes(), b'original')

    def test_bad_member_hash_leaves_checkout_untouched(self):
        self.manifest['inputs'][0]['sha256'] = '0' * 64
        with self.assertRaises(ValueError):
            provenance.stage(self.root, self.manifest, self.archive)
        self.assertEqual(self.dest.read_bytes(), b'original')

    def test_missing_member_leaves_checkout_untouched(self):
        self.manifest['inputs'].append(dict(self.manifest['inputs'][0],
            archive_member='missing.o', destination='missing.o'))
        with self.assertRaises(ValueError):
            provenance.stage(self.root, self.manifest, self.archive)
        self.assertEqual(self.dest.read_bytes(), b'original')

    def test_reject_path_escape_and_symlink(self):
        for name in ['../escape', '/absolute']:
            with self.assertRaises(ValueError):
                provenance.contained(self.root, name)
        (self.root / 'link').symlink_to(self.dest)
        with self.assertRaises(ValueError):
            provenance.contained(self.root, 'link')

    def test_editing_status_cannot_authorize_release(self):
        with self.assertRaises(ValueError):
            provenance.release_gate({'coverage': 'complete'},
                {'publishable_firmware': True, 'blockers': []})

    def test_manifest_destinations_unique_and_model_specific(self):
        manifest = json.loads((provenance.HERE / 'inputs.json').read_text())
        paths = [r['destination'] for r in manifest['inputs']]
        self.assertEqual(len(paths), len(set(paths)))
        self.assertTrue(all('GT-BE98_PRO' in p or 'gt-be98_pro' in p for p in paths))


if __name__ == '__main__':
    unittest.main()
