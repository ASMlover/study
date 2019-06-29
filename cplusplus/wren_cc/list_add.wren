
var a = []
a.add(1)
io.write(a) // expect: [1]
a.add(2)
io.write(a) // expect: [1, 2]
a.add(3)
io.write(a) // expect: [1, 2, 3]
a.add("s")
io.write(a) // expect: [1, 2, 3, "s"]

io.write(a.add(4)) // expect: 4
