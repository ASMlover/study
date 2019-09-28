
class FibIterator {
  this new() {
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
  iterate(iterator) {
    if (iterator == nil) return FibIterator.new()
    iterator.iterate
    return iterator
  }

  iterValue(iterator) { iterator.value }
}

var squareFib = Fib.new().map{|fib| fib * fib }
var iterator = nil

IO.print(squareFib is Sequence) // expect: true
IO.print(squareFib) // expect: instance of MapSequence

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iterValue(iterator)) // expect:  0

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iterValue(iterator)) // expect:  1

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iterValue(iterator)) // expect:  1

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iterValue(iterator)) // expect:  4

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iterValue(iterator)) // expect:  9
