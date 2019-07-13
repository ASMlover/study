
var a = "a"
var b = "b"
var c = "c"

a = b = c
IO.write(a) // expect: c
IO.write(b) // expect: c
IO.write(c) // expect: c
