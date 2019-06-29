
var a = []
a.insert(0, 1)
io.write(a) // expect: [1]

var b = [1, 2, 3]
b.insert(0, 4)
io.write(b) // expect: [4, 1, 2, 3]

var c = [1, 2, 3]
c.insert(1, 4)
io.write(c) // expect: [1, 4, 2, 3]

var d = [1, 2, 3]
d.insert(2, 4)
io.write(d) // expect: [1, 2, 4, 3]

var e = [1, 2, 3]
e.insert(3, 4)
io.write(e) // expect: [1, 2, 3, 4]

var aa = [1, 2, 3]
aa.insert(-4, 4)
io.write(aa) // expect: [4, 1, 2, 3]

var bb = [1, 2, 3]
bb.insert(-3, 4)
io.write(bb) // expect: [1, 4, 2, 3]

var cc = [1, 2, 3]
cc.insert(-2, 4)
io.write(cc) // expect: [1, 2, 4, 3]

var dd = [1, 2, 3]
dd.insert(-1, 4)
io.write(dd) // expect: [1, 2, 3, 4]

io.write([1, 2].insert(0, 3)) // expect: 3
