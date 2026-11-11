import unittest
from sw_gso_model import Queue


class QueueContractTests(unittest.TestCase):
    def test_unavailable_retains_packet(self):
        q = Queue(2)
        self.assertEqual(q.enqueue('p', 'd', 'a', 'b', available=False), 16)
        self.assertEqual(list(q.dropped), [])
        self.assertIsNone(q.take_for_test())

    def test_full_consumes_overflow_without_overwriting(self):
        q = Queue(2)
        for packet in range(3):
            self.assertEqual(q.enqueue(packet, 'dev', 'segment', 'xmit'), 0)
        self.assertEqual(list(q.dropped), [2])
        self.assertEqual(q.take_for_test(), ('dev', 0, 'segment', 'xmit'))
        self.assertEqual(q.take_for_test(), ('dev', 1, 'segment', 'xmit'))
        self.assertIsNone(q.take_for_test())

    def test_counter_wrap_preserves_fifo(self):
        q = Queue(4)
        q.head = q.tail = 65534
        for packet in range(4):
            q.enqueue(packet, 'd', 'a', 'b')
        self.assertEqual(q.head, 2)
        self.assertEqual([q.take_for_test()[1] for _ in range(4)], list(range(4)))
        self.assertEqual(q.head, q.tail)

    def test_inactive_consumes_packet(self):
        q = Queue(2)
        q.active = False
        self.assertEqual(q.enqueue('p', 'd', 'a', 'b'), 0)
        self.assertEqual(list(q.dropped), ['p'])

    def test_invalid_capacity(self):
        for n in (0, -1, 3, 32768):
            with self.assertRaises(ValueError):
                Queue(n)
