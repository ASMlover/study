
var s = "abcd"
IO.print(s.iterate(nil)) // expect: 0
IO.print(s.iterate(0)) // expect: 1
IO.print(s.iterate(1)) // expect: 2
IO.print(s.iterate(2)) // expect: 3
IO.print(s.iterate(3)) // expect: false

// out of bounds
IO.print(s.iterate(123)) // expect: false
IO.print(s.iterate(-1)) // expect: false

// nothing to iterate in an empty string
IO.print("".iterate(nil)) // expect: false
