
var map = {
  "one": 1,
  "two": 2,
  "three": 3
}

IO.print(map.len) // expect: 3
IO.print(map.remove("two")) // expect: 2
IO.print(map.len) // expect: 2
IO.print(map.remove("three")) // expect: 3
IO.print(map.len) // expect: 1

// remove an already removed entry
IO.print(map.remove("two")) // expect: nil
IO.print(map.len) // expect: 1

IO.print(map.remove("one")) // expect: 1
IO.print(map.len) // expect: 0
