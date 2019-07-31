
// ordered range
IO.print((2..5).max) // expect: 5
IO.print((3..3).max) // expect: 3
IO.print((0..3).max) // expect: 3
IO.print((-5..3).max) // expect: 3
IO.print((-5..-2).max) // expect: -2

// backwards range
IO.print((5..2).max) // expect: 5
IO.print((3..0).max) // expect: 3
IO.print((3..-5).max) // expect: 3
IO.print((-2..-5).max) // expect: -2

// exclusive ordered range
IO.print((2...5).max) // expect: 4
IO.print((3...3).max) // expect: 3
IO.print((0...3).max) // expect: 2
IO.print((-5...3).max) // expect: 2
IO.print((-5...-2).max) // expect: -3

// exclusive backwards range
IO.print((5...2).max) // expect: 5
IO.print((3...0).max) // expect: 3
IO.print((3...-5).max) // expect: 3
IO.print((-2...-5).max) // expect: -2
