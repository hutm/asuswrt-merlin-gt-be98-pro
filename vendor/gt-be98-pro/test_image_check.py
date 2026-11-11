import hashlib
import struct
import unittest
from image_check import inspect


def fixture(payload=b'payload', include_hash=True):
    strings = b'data\0algo\0value\0'
    def word(n):
        return struct.pack('>I', n)
    def padded(b):
        return b + b'\0' * (-len(b) % 4)
    def node(name):
        return word(1) + padded(name.encode() + b'\0')
    def prop(offset, value):
        return word(3) + word(len(value)) + word(offset) + padded(value)
    tree = node('') + node('images') + node('rootfs') + prop(0, payload)
    if include_hash:
        tree += node('hash-1') + prop(5, b'sha256\0') + prop(10, hashlib.sha256(payload).digest()) + word(2)
    tree += word(2) * 3 + word(9)
    header = struct.pack('>10I', 0xd00dfeed, 56 + len(tree) + len(strings),
                         56, 56 + len(tree), 40, 17, 16, 0, len(strings), len(tree))
    return header + b'\0' * 16 + tree + strings


class IntegrityTests(unittest.TestCase):
    def test_valid_payload(self):
        self.assertEqual(inspect(fixture())['rootfs']['sha256'], hashlib.sha256(b'payload').hexdigest())

    def test_corrupt_payload(self):
        with self.assertRaises(ValueError):
            inspect(fixture().replace(b'payload', b'badload'))

    def test_missing_hash(self):
        with self.assertRaises(ValueError):
            inspect(fixture(include_hash=False))

    def test_truncation(self):
        with self.assertRaises(ValueError):
            inspect(fixture()[:-1])
