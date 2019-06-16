
var a = "a"
var b = "b"
a = b = "chain"
io.write(a) // expect: chain
io.write(b) // expect: chain

var c = a = "var"
io.write(a) // expect: var
io.write(c) // expect: var
