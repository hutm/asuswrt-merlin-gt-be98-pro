import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('check_upstream', Path(__file__).with_name('check-upstream.py'))
upstream = importlib.util.module_from_spec(spec)
spec.loader.exec_module(upstream)


class UpstreamTests(unittest.TestCase):
    def test_numeric_order_excludes_prereleases_and_prefers_commit(self):
        tags = upstream.stable_tags('''a refs/tags/3006.102.9
b refs/tags/3006.102.10
c refs/tags/3006.102.10^{}
d refs/tags/3006.102.10_2
e refs/tags/3006.102.11-beta1
f refs/tags/3006.102.11-alpha1
''')
        self.assertEqual(tags['3006.102.10'][2], 'c')
        self.assertEqual(max(tags, key=lambda n: tags[n][:2]), '3006.102.10_2')
        self.assertEqual(len(tags), 3)


if __name__ == '__main__':
    unittest.main()
