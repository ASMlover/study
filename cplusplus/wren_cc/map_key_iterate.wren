
var a = {"one": 1, "two": 2, "three": 3, "four": 4}.keys

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

var prev = -1
var iterator = a.iterate(nil)
while (iterator) {
  IO.print(iterator > prev)
  IO.print(iterator is Numeric)
  prev = iterator
  iterator = a.iterate(iterator)
}

// first entry
//  expect: true
//  expect: true
// second entry
//  expect: true
//  expect: true
// third entry
//  expect: true
//  expect: true
// fourth entry
//  expect: true
//  expect: true

// out of bounds
IO.print(a.iterate(16)) // expect: false
IO.print(a.iterate(-1)) // expect: false

// nothing to iterate in an empty map
IO.print({}.keys.iterate(nil)) // expect: false
