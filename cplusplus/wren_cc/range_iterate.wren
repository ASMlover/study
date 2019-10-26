
// inclusive
var range = 1..3
IO.print(range.iterate(nil)) // expect: 1
IO.print(range.iterate(1)) // expect: 2
IO.print(range.iterate(2)) // expect: 3
IO.print(range.iterate(3)) // expect: false
IO.print(range.iterate(4)) // expect: false

// exclusive
range = 1...3
IO.print(range.iterate(nil)) // expect: 1
IO.print(range.iterate(1)) // expect: 2
IO.print(range.iterate(2)) // expect: false

// negative inclusive range
range = 3..1
IO.print(range.iterate(nil)) // expect: 3
IO.print(range.iterate(3)) // expect: 2
IO.print(range.iterate(2)) // expect: 1
IO.print(range.iterate(1)) // expect: false

// negative exclusive range
range = 3...1
IO.print(range.iterate(nil)) // expect: 3
IO.print(range.iterate(3)) // expect: 2
IO.print(range.iterate(2)) // expect: false

// empty inclusive range
range = 1..1
IO.print(range.iterate(nil)) // expect: 1
IO.print(range.iterate(1)) // expect: false

// empty exclusive range
range = 1...1
IO.print(range.iterate(nil)) // expect: false
