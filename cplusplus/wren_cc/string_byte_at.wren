
var s = "something"

IO.print(s.byteAt(0)) // expect: 115
IO.print(s.byteAt(1)) // expect: 111
IO.print(s.byteAt(2)) // expect: 109
IO.print(s.byteAt(3)) // expect: 101
IO.print(s.byteAt(4)) // expect: 116
IO.print(s.byteAt(5)) // expect: 104
IO.print(s.byteAt(6)) // expect: 105
IO.print(s.byteAt(7)) // expect: 110
IO.print(s.byteAt(8)) // expect: 103

IO.print(s.byteAt(-9)) // expect: 115
IO.print(s.byteAt(-8)) // expect: 111
IO.print(s.byteAt(-7)) // expect: 109
IO.print(s.byteAt(-6)) // expect: 101
IO.print(s.byteAt(-5)) // expect: 116
IO.print(s.byteAt(-4)) // expect: 104
IO.print(s.byteAt(-3)) // expect: 105
IO.print(s.byteAt(-2)) // expect: 110
IO.print(s.byteAt(-1)) // expect: 103

IO.print("\0".byteAt(0)) // expect: 0
