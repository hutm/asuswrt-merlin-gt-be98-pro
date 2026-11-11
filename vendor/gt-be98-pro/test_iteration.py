import importlib.util
from pathlib import Path
import tempfile
import time
import unittest

spec = importlib.util.spec_from_file_location('iterate', Path(__file__).with_name('iterate.py'))
iteration = importlib.util.module_from_spec(spec)
spec.loader.exec_module(iteration)


class ArtifactTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.images = self.root / 'images'
        self.images.mkdir()
        self.output = self.root / 'output'
        self.output.mkdir()
        self.name = 'GT-BE98_PRO_3006_102.9_0_nand_squashfs.pkgtb'

    def test_missing_and_stale_images_fail(self):
        with self.assertRaises(ValueError):
            iteration.collect_images(self.images, self.output, 0)
        (self.images / self.name).write_bytes(b'old image')
        with self.assertRaises(ValueError):
            iteration.collect_images(self.images, self.output, time.time_ns() + 10**9)

    def test_loader_not_archived(self):
        (self.images / self.name).write_bytes(b'candidate')
        (self.images / self.name.replace('.pkgtb', '_loader.pkgtb')).write_bytes(b'loader')
        result = iteration.collect_images(self.images, self.output, 0)
        self.assertEqual(list(result), [self.name])
        self.assertEqual(list(self.output.iterdir()), [self.output / self.name])
        self.assertEqual(result[self.name]['sha256'], iteration.sha(self.output / self.name))

    def test_ambiguous_versions_fail(self):
        (self.images / self.name).write_bytes(b'one')
        (self.images / self.name.replace('102.9', '102.8')).write_bytes(b'two')
        with self.assertRaises(ValueError):
            iteration.collect_images(self.images, self.output, 0)

    def test_symlink_not_archived(self):
        (self.root / 'other').write_bytes(b'other')
        (self.images / self.name).symlink_to(self.root / 'other')
        with self.assertRaises(ValueError):
            iteration.collect_images(self.images, self.output, 0)


class RunEvidenceTests(unittest.TestCase):
    def test_build_failure_is_recorded_without_candidate(self):
        from unittest.mock import patch
        import json
        import subprocess
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            kit = root / 'kit'
            kit.mkdir()
            for name in ('Dockerfile', 'scripts/20-stage-toolchain.sh',
                         'scripts/30-build-firmware.sh', 'openssl-3.5-static-atomic.patch'):
                path = kit / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('fixture')
            responses = [str(root), 'test-head', 'tracked-change']
            with patch.object(iteration, 'preflight'), \
                 patch.object(iteration.subprocess, 'check_output', side_effect=responses), \
                 patch.object(iteration, 'run', side_effect=subprocess.CalledProcessError(7, 'build')):
                with self.assertRaises(subprocess.CalledProcessError):
                    iteration.build(kit, root / 'runs')
            results = list((root / 'runs').glob('*/result.json'))
            self.assertEqual(len(results), 1)
            record = json.loads(results[0].read_text())
            self.assertEqual(record['state'], 'failed')
            self.assertFalse(record['installable'])
            self.assertEqual(record['images'], {})


if __name__ == '__main__':
    unittest.main()
