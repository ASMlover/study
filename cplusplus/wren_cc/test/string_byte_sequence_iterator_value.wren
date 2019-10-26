
var bytes = "some".bytes

IO.print(bytes.iterValue(0)) // expect: 115
IO.print(bytes.iterValue(1)) // expect: 111
IO.print(bytes.iterValue(2)) // expect: 109
IO.print(bytes.iterValue(3)) // expect: 101

IO.print(bytes.iterValue(-4)) // expect: 115
IO.print(bytes.iterValue(-3)) // expect: 111
IO.print(bytes.iterValue(-2)) // expect: 109
IO.print(bytes.iterValue(-1)) // expect: 101
