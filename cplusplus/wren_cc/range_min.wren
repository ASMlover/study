
// ordered range
IO.print((2..5).min) // expect: 2
IO.print((3..3).min) // expect: 3
IO.print((0..3).min) // expect: 0
IO.print((-5..3).min) // expect: -5
IO.print((-5..-2).min) // expect: -5

// backwards range
IO.print((5..2).min) // expect: 2
IO.print((3..0).min) // expect: 0
IO.print((3..-5).min) // expect: -5
IO.print((-2..-5).min) // expect: -5

// exclusive ordered range
IO.print((2...5).min) // expect: 2
IO.print((3...3).min) // expect: 3
IO.print((0...3).min) // expect: 0
IO.print((-5...3).min) // expect: -5
IO.print((-5...-2).min) // expect: -5

// exclusive backwards range
IO.print((5...2).min) // expect: 2
IO.print((3...0).min) // expect: 0
IO.print((3...-5).min) // expect: -5
IO.print((-2...-5).min) // expect: -5
