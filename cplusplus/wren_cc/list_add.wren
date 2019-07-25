
var a = []
a.add(1)
IO.print(a) // expect: [1]
a.add(2)
IO.print(a) // expect: [1, 2]
a.add(3)
IO.print(a) // expect: [1, 2, 3]
a.add("s")
IO.print(a) // expect: [1, 2, 3, "s"]

IO.print(a.add(4)) // expect: 4
