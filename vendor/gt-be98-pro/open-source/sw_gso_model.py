"""Executable specification of observed enqueue behavior, not a kernel driver.

Observations: sw_gso.ko sha256
7fba345b7959ab34f7a0d39cafb572d1b8ecd58dc2f1cf8d2ee040e2ac8aeb52.
Only models the queue portion; assumes caller serialization, no kernel pointers.
"""
from collections import deque


class Queue:
    def __init__(self, capacity):
        # Masked indexing requires power-of-two capacity. Signed 16-bit occupancy
        # implies a conservative bound; original initialization limits unresolved.
        if capacity < 1 or capacity >= 32768 or capacity & (capacity - 1):
            raise ValueError('capacity must be a power of two below 32768')
        self.capacity = capacity
        self.head = self.tail = 0
        self.slots = [None] * capacity
        self.active = True
        self.dropped = deque()

    def enqueue(self, packet, device, callback1, callback2, available=True):
        # 0x2dc: positive 16, packet left with caller when worker unavailable.
        if not available:
            return 16
        # 0x210..0x238: full ring or zero active field consumes/drops packet.
        if (self.head - self.tail) & 0xffff == self.capacity or not self.active:
            self.dropped.append(packet)
            return 0
        # 0x25c..0x268: stored order is device, packet, callback1, callback2.
        self.slots[self.head & (self.capacity - 1)] = (device, packet, callback1, callback2)
        self.head = (self.head + 1) & 0xffff
        return 0

    def take_for_test(self):
        # Test harness consumer, NOT a reconstruction of sw_gso_task.
        if self.head == self.tail:
            return None
        index = self.tail & (self.capacity - 1)
        result = self.slots[index]
        self.slots[index] = None
        self.tail = (self.tail + 1) & 0xffff
        return result
