
var a = [1, 2, 3]
var gt1 = fn(x) { return x > 1 }
var gt10 = fn(x) { return x > 10 }
var b = a.where(gt1)
var c = a.where(gt10)

IO.print(b) // expect: [2 ,3]
IO.print(c) // expect: []
