
var a = 1..3
var b = a.where {|x| x > 1 }.list
IO.print(b) // expect: [2, 3]

var c = a.where {|x| x > 10 }.list
IO.print(c) // expect: []
