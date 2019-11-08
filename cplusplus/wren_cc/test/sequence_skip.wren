
class TestSequence is Sequence {
  construct new() {}

  iterate(iterator) {
    if (iterator == nil) return 1
    if (iterator == 3) return false
    return iterator + 1
  }

  iterValue(iterator) { iterator }
}

var test = TestSequence.new().skip(0)

IO.print(test is Sequence) // expect: true
IO.print(test) // expect: instance of SkipSequence

IO.print(test.toList) // expect: [1, 2, 3]

IO.print(test.skip(1).toList) // expect: [2, 3]

IO.print(test.skip(4).isEmpty) // expect: true

IO.print(test.skip(-10).toList) // expect: [1, 2, 3]
