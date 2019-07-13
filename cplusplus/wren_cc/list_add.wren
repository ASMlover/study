
var a = []
a.add(1)
IO.write(a) // expect: [1]
a.add(2)
IO.write(a) // expect: [1, 2]
a.add(3)
IO.write(a) // expect: [1, 2, 3]
a.add("s")
IO.write(a) // expect: [1, 2, 3, "s"]

IO.write(a.add(4)) // expect: 4
