
var a = [1, 2, 3]
var b = a.where {|x| x > 1 }
IO.print(b) // expect: [2 ,3]
var c = a.where {|x| x > 10 }
IO.print(c) // expect: []
