
var a = "a"
var b = "b"
a = b = "chain"
IO.print(a) // expect: chain
IO.print(b) // expect: chain

var c = a = "var"
IO.print(a) // expect: var
IO.print(c) // expect: var
