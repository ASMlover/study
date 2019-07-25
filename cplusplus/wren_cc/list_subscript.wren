
var list = ["a", "b", "c", "d", 555]
IO.print(list[0]) // expect: a
IO.print(list[1]) // expect: b
IO.print(list[2]) // expect: c
IO.print(list[3]) // expect: d
IO.print(list[4]) // expect: 555

IO.print(list[-5]) // expect: a
IO.print(list[-4]) // expect: b
IO.print(list[-3]) // expect: c
IO.print(list[-2]) // expect: d
IO.print(list[-1]) // expect: 555
