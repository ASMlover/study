
var a = "a"
var b = "b"
a = b = "chain"
IO.write(a) // expect: chain
IO.write(b) // expect: chain

var c = a = "var"
IO.write(a) // expect: var
IO.write(c) // expect: var
