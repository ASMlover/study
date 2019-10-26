
// ordered range
IO.print((2..5).to) // expect: 5
IO.print((3..3).to) // expect: 3
IO.print((0..3).to) // expect: 3
IO.print((-5..3).to) // expect: 3
IO.print((-5..-2).to) // expect: -2

// backwards range
IO.print((5..2).to) // expect: 2
IO.print((3..0).to) // expect: 0
IO.print((3..-5).to) // expect: -5
IO.print((-2..-5).to) // expect: -5

// exclusive ordered range
IO.print((2...5).to) // expect: 5
IO.print((3...3).to) // expect: 3
IO.print((0...3).to) // expect: 3
IO.print((-5...3).to) // expect: 3
IO.print((-5...-2).to) // expect: -2

// exclusive backwards range
IO.print((5...2).to) // expect: 2
IO.print((3...0).to) // expect: 0
IO.print((3...-5).to) // expect: -5
IO.print((-2...-5).to) // expect: -5
