
IO.print([].isEmpty) // expect: true
IO.print([1].isEmpty) // expect: false

class InfiniteSequence is Sequence {
  iterate(iterator) { true }
  iterValue(iterator) { iterator }
}

IO.print((new InfiniteSequence).isEmpty) // expect: false
