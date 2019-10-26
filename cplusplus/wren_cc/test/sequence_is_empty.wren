
IO.print([].isEmpty) // expect: true
IO.print([1].isEmpty) // expect: false

class InfiniteSequence is Sequence {
  construct new() {}
  iterate(iterator) { true }
  iterValue(iterator) { iterator }
}

IO.print(InfiniteSequence.new().isEmpty) // expect: false
