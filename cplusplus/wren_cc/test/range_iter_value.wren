
var range = 1..3
IO.print(range.iterValue(1)) // expect: 1
IO.print(range.iterValue(2)) // expect: 2
IO.print(range.iterValue(3)) // expect: 3

// does not bother to bounds check
IO.print(range.iterValue(-2)) // expect: -2
IO.print(range.iterValue(5)) // expect: 5

// type check
IO.print(range.iterValue("s")) // expect: s
