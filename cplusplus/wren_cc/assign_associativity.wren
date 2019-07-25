
var a = "a"
var b = "b"
var c = "c"

a = b = c
IO.print(a) // expect: c
IO.print(b) // expect: c
IO.print(c) // expect: c
