
var a = ["one", "two", 3, 4]
IO.print(a.iterate(nil)) // expect: 0
IO.print(a.iterate(0)) // expect: 1
IO.print(a.iterate(1)) // expect: 2
IO.print(a.iterate(2)) // expect: 3
IO.print(a.iterate(3)) // expect: false
IO.print(a.iterate(-1)) // expect: false

// nothing to iterate in an empty list
IO.print([].iterate(nil)) // expect: false
