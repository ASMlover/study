
class TestSequence is Sequence {
  iterate(iterator) {
    if (iterator == nil) return 1
    if (iterator == 3) return false
    return iterator + 1
  }

  iterValue(iterator) { iterator }
}

IO.print(TestSequence.new().toList) // expect: [1, 2, 3]
