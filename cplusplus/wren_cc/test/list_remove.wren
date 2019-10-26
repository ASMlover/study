
var a = [1, 2, 3]
a.remove(0)
IO.print(a) // expect: [2, 3]

var b = [1, 2, 3]
b.remove(1)
IO.print(b) // expect: [1, 3]

var c = [1, 2, 3]
c.remove(2)
IO.print(c) // expect: [1, 2]

var aa = [1, 2, 3]
aa.remove(-3)
IO.print(aa) // expect: [2, 3]

var bb = [1, 2, 3]
bb.remove(-2)
IO.print(bb) // expect: [1, 3]

var cc = [1, 2, 3]
cc.remove(-1)
IO.print(cc) // expect: [1, 3]

IO.print([1, 2, 3].remove(1)) // expect: 2
