
var a = "a"
var b = "b"
var c = "c"

a = b = c
io.write(a) // expect: c
io.write(b) // expect: c
io.write(c) // expect: c
