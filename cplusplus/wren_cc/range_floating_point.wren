
// ordered range
IO.print((2..5).from) // expect: 2
IO.print((3..3).from) // expect: 3
IO.print((0..3).from) // expect: 0
IO.print((-5..3).from) // expect: -5
IO.print((-5..-2).from) // expect: -5

// backwards range
IO.print((5..2).from) // expect: 5
IO.print((3..0).from) // expect: 3
IO.print((3..-5).from) // expect: 3
IO.print((-2..-5).from) // expect: -2

// exclusive ordered range
IO.print((2...5).from) // expect: 2
IO.print((3...3).from) // expect: 3
IO.print((0...3).from) // expect: 0
IO.print((-5...3).from) // expect: -5
IO.print((-5...-2).from) // expect: -5

// exclusive backwards range
IO.print((5..2).from) // expect: 5
IO.print((3..0).from) // expect: 3
IO.print((3..-5).from) // expect: 3
IO.print((-2..-5).from) // expect: -2
