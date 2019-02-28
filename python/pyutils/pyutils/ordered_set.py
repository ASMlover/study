#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2019 ASMlover. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list ofconditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materialsprovided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
import itertools as it
from collections import deque, MutableSet, Sequence

SLICE_ALL = slice(None)

def is_iterable(obj):
    return hasattr(obj, '__iter__') and not isinstance(obj, (str, tuple))

class OrderedSet(MutableSet, Sequence):
    """An OrderedSet is a custom MutableSet that remembers its orders, so that
    every entry has an index that can be looked up.

    Example:
        >>> OrderedSet([1, 1, 2, 1, 3, 3, 2])
        OrderedSet([1, 2, 3])
    """

    def __init__(self, iterable=None):
        self.items = []
        self.map = {}
        if iterable is not None:
            self |= iterable

    def __len__(self):
        """Returns the number of unique elements in the ordered set.

        Example:
            >>> len(OrderedSet())
            0
            >>> len(OrderedSet([1, 2]))
            2
        """
        return len(self.items)

    def __getitem__(self, index):
        """Get the item at a given index.

        Example:
            >>> s = OrderedSet([1, 2, 3])
            >>> s[1]
            2
        """
        if isinstance(index, slice) and index == SLICE_ALL:
            return self.copy()
        elif hasattr(index, '__index__') or isinstance(index, slice):
            result = self.items[index]
            if isinstance(result, list):
                return self.__class__(result)
            else:
                return result
        elif is_iterable(index):
            return [self.items[i] for i in index]
        else:
            raise TypeError('Do not know how to index an OrderedSet by %r' % index)

    def copy(self):
        """Returns a shallow copy of this object.

        Example:
            >>> this = OrderedSet([1, 2, 3])
            >>> otehr = this.copy()
            >>> this == other
            True
            >>> this is other
            False
        """
        return self.__class__(self)

    def __getstate__(self):
        if len(self) == 0:
            return (None,)
        else:
            return list(self)

    def __setstate__(self, state):
        if state == (None,):
            self.__init__([])
        else:
            self.__init__(state)

    def __contains__(self, key):
        """Test if the item is in this ordered set.

        Example:
            >>> 1 in OrderedSet([1, 2, 3])
            True
            >>> 5 in OrderedSet([1, 2, 3])
            False
        """
        return key in self.map

    def add(self, key):
        """Add `key` as an item to this OrderedSet, then return it's index.
        If `key` is already in the OrderedSet, return the index it already had.

        Example:
            >>> s = OrderedSet()
            >>> s.add(3)
            0
            >>> print s
            OrderedSet([3])
        """
        if key not in self.map:
            self.map[key] = len(self.items)
            self.items.append(key)
        return self.map[key]

    def pop(self):
        """Remote and return the last element from the ordered set.
        Raises KeyError if the set is empty.

        Example:
            >>> s = OrderedSet([1, 2, 3])
            >>> s.pop()
            3
        """
        if not self.items:
            raise KeyError('set is empty')
        e = self.items[-1]
        del self.items[-1]
        del self.map[e]
        return e

    def discard(self, key):
        """Remove an element. Do not raise an exception if absent.

        Example:
            >>> s = OrderedSet([1, 2, 3])
            >>> s.discard(2)
            >>> print s
            OrderedSet([1, 3])
            >>> s.discard(2)
            >>> print s
            OrderedSet([1, 3])
        """
        if key in self:
            i = self.map[key]
            del self.items[i]
            del self.map[key]
            for k, v in self.map.iteritems():
                if v >= i:
                    self.map[k] = v - 1

    def clear(self):
        """Remove all items from this OrderedSet."""
        del self.items[:]
        self.map.clear()

    def __iter__(self):
        """
        Example:
            >>> list(iter(OrderedSet([1, 2, 3])))
            [1, 2, 3]
        """
        return iter(self.items)

    def __reversed__(self):
        """
        Example:
            >>> list(reversed(OrderedSet([1, 2, 3])))
            [3, 2, 1]
        """
        return reversed(self.items)

    def __repr__(self):
        if not self:
            return '%s()' % (self.__class__.__name__)
        return '%s(%r)' % (self.__class__.__name__, list(self))

    def __eq__(self, other):
        """Returns True if the containers have the same item, if other is a
        Sequence, then other is checked, otherwise it is ignored.

        Example:
            >>> s = OrderedSet([1, 2, 3])
            >>> s == [1, 2, 3]
            True
            >>> s == [1, 3, 2]
            False
            >>> s == OrderedSet([1, 2, 3])
            True
        """
        if isinstance(other, (Sequence, deque)):
            return list(self) == list(other)
        try:
            other_as_set = set(other)
        except TypeError:
            return False
        else:
            return set(self) == other_as_set

    def union(self, *sets):
        """Combines all unique items. Each items order is defined by it's
        first appearence.

        Example:
            >>> s = OrderedSet.union(OrderedSet([3, 1, 4, 5]), [1, 3], [2, 0])
            >>> print s
            OrderedSet([3, 1, 4, 5, 2, 0])
            >>> s.union([8, 9])
            OrderedSet([3, 1, 4, 5, 2, 0, 8, 9])
            >>> s | {10}
            OrderedSet([3, 1, 4, 5, 2, 0, 10])
        """
        cls = self.__class__ if isinstance(self, OrderedSet) else OrderedSet
        containers = map(list, it.chain([self], sets))
        items = it.chain.from_iterable(containers)
        return cls(items)

    def intersection(self, *sets):
        """Returns elements in common between all sets. Order is defined
        only by the first set.

        Example:
            >>> s = OrderedSet.intersection(OrderedSet([1, 2, 3, 4]), [1, 2, 3])
            >>> print s
            OrderedSet([1, 2, 3])
            >>> s.intersection([2, 4, 5], [1, 2, 3, 4])
            OrderedSet([2])
            >>> s.intersection()
            OrderedSet([1, 2, 3])
        """
        cls = self.__class__ if isinstance(self, OrderedSet) else OrderedSet
        if sets:
            common = set.intersection(*map(set, sets))
            items = (item for item in self if item in common)
        else:
            items = self
        return cls(items)

    def __and__(self, other):
        return self.intersection(other)

    def difference(self, *sets):
        """Returns all elements that are in this set but not the others.

        Example:
            >>> OrderedSet([1, 2, 3]).difference(OrderedSet([2]))
            OrderedSet([1, 3])
            >>> OrderedSet([1, 2, 3]).difference(OrderedSet([2]), OrderedSet([3]))
            OrderedSet([1])
            >>> OrderedSet([1, 2, 3]) - OrderedSet([2])
            OrderedSet([1, 3])
            >>> OrderedSet([1, 2, 3]).difference()
            OrderedSet([1, 2, 3])
        """
        cls = self.__class__
        if sets:
            common = set.union(*map(set, sets))
            items = (item for item in self if item in common)
        else:
            items = self
        return cls(items)

    def issubset(self, other):
        """Check whether another set contains this set.

        Example:
            >>> OrderedSet([1, 2, 3]).issubset({1, 2})
            False
            >>> OrderedSet([1, 2, 3]).issubset({1, 2, 3, 4})
            True
        """
        if len(self) > len(other):
            return False
        return all(item in other for item in self)

    def issuperset(self, other):
        """Check whether this set contains another set.

        Example:
            >>> OrderedSet([1, 2]).issuperset({1, 2, 3})
            False
            >>> OrderedSet([1, 2, 3, 4]).issuperset({1, 2, 3})
            True
        """
        if len(self) < len(other):
            return False
        return all(item in self for item in other)

    def symmetric_difference(self, other):
        """Return ths symmetric difference of two OrderedSet as a new set.

        Example:
            >>> this = OrderedSet([1, 3, 4, 5, 7])
            >>> other = OrderedSet([1, 2, 3, 7, 9])
            >>> this.symmetric_difference(other)
            OrderedSet([4, 5, 2, 9])
        """
        cls = self.__class__ if isinstance(self, OrderedSet) else OrderedSet
        diff1 = cls(self).difference(other)
        diff2 = cls(other).difference(self)
        return diff1.union(diff2)

    def update_items(self, items):
        self.items = items
        self.map = {item: index for index, item in enumerate(items)}

    def difference_update(self, *sets):
        """Update this OrderedSet to remove items from one or more other sets.

        Example:
            >>> s = OrderedSet([1, 2, 3])
            >>> s.difference_update(OrderedSet([2, 4]))
            >>> print s
            OrderedSet([1, 3])
            >>> s = OrderedSet([1, 2, 3, 4, 5])
            >>> s.difference_update(OrderedSet([2, 4]), OrderedSet([1, 4, 6]))
            >>> print s
            OrderedSet([3, 5])
        """
        items_to_remove = set()
        for other in sets:
            items_to_remove |= set(other)
        self.update_items([item for item in self.items if item not in items_to_remove])

    def intersection_update(self, other):
        """Update this OrderedSet to keep only item in another set, preserving
        their order in this set.

        Example:
            >>> this = OrderedSet([1, 3, 4, 5, 7])
            >>> other = OrderedSet([1, 2, 3, 7, 9])
            >>> this.intersection_update(other)
            >>> print this
            OrderedSet([1, 3, 7])
        """
        other = set(other)
        self.update_items([item for item in self.items if item in other])

    def symmetric_difference_update(self, other):
        """Update this OrderedSet to remove items from another set, then
        add items from the other set that were not present in this set.

        Example:
            >>> this = OrderedSet([1, 3, 4, 5, 7])
            >>> other = OrderedSet([1, 2, 3, 7, 9])
            >>> this.symmetric_difference_update(other)
            >>> print this
            OrderedSet([4, 5, 2, 9])
        """
        items_to_add = [item for item in other if item in self]
        items_to_remove = set(other)
        self.update_items([item for item in self.items if item not in items_to_remove] + items_to_add)
