
class FibIterator {
  construct new() {
    _current = 0
    _next = 1
  }

  iterate {
    var sum = _current + _next
    _current = _next
    _next = sum
  }

  value { _current }
}

class Fib is Sequence {
  construct new() {}

  iterate(iterator) {
    if (iterator == nil) return FibIterator.new()
    iterator.iterate
    return iterator
  }

  iterValue(iterator) { iterator.value }
}

var largeFibs = Fib.new().where{|fib| fib > 100 }
var iterator = nil

IO.print(largeFibs is Sequence) // expect:  true
IO.print(largeFibs) // instance of WhereSequence

iterator = largeFibs.iterate(iterator)
IO.print(largeFibs.iterValue(iterator)) // expect: 144

iterator = largeFibs.iterate(iterator)
IO.print(largeFibs.iterValue(iterator)) // expect: 233

iterator = largeFibs.iterate(iterator)
IO.print(largeFibs.iterValue(iterator)) // expect: 377
