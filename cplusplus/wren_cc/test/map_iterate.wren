
var a = {"one": 1, "two": 2, "three": 3, "four": 4}

IO.print(a.iterate(nil) is Numeric) // expect: true
IO.print(a.iterate(nil) >= 0) // expect: true

IO.print(a.iterate(0) is Numeric) // expect: true
IO.print(a.iterate(0) > 0) // expect: true
IO.print(a.iterate(1) is Numeric) // expect: true
IO.print(a.iterate(1) > 0) // expect: true
IO.print(a.iterate(2) is Numeric) // expect: true
IO.print(a.iterate(2) > 0) // expect: true
IO.print(a.iterate(3) is Numeric) // expect: true
IO.print(a.iterate(3) > 0) // expect: true

var previous = -1
var iterator = a.iterate(nil)
while (iterator) {
  IO.print(iterator > previous) // expect: true
  IO.print(iterator is Numeric) // expect: true
  previous = iterator
  iterator = a.iterate(iterator)
}
// run 4 times

IO.print(a.iterate(16)) // expect: false
IO.print(a.iterate(-1)) // expect: false

IO.print({}.iterate(nil)) // expect: false
