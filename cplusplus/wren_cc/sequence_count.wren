
class TestSequence is Sequence {
  iterate(iterator) {
    if (iterator == nil) return 1
    if (iterator == 10) return false
    return iterator + 1
  }

  iterValue(iterator) { iterator }
}

IO.print(TestSequence.new().count) // expect: 10
