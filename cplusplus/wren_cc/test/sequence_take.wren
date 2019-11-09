
class TestSequence is Sequence {
  construct new() {}

  iterate(iterator) {
    if (iterator == nil) return 1
    if (iterator == 3) return false
    return iterator + 1
  }

  iterValue(iterator) { iterator }
}

var test = TestSequence.new().take(3)

IO.print(test is Sequence) // expect: true
IO.print(test) // expect: instance of TakeSequence

IO.print(test.take(0).isEmpty) // expect: true

IO.print(test.take(1).toList) // expect: [1]

IO.print(test.take(4).toList) // expect: [1, 2, 3]
