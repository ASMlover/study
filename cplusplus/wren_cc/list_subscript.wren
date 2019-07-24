
var list = ["a", "b", "c", "d", 555]
IO.write(list[0]) // expect: a
IO.write(list[1]) // expect: b
IO.write(list[2]) // expect: c
IO.write(list[3]) // expect: d
IO.write(list[4]) // expect: 555

IO.write(list[-5]) // expect: a
IO.write(list[-4]) // expect: b
IO.write(list[-3]) // expect: c
IO.write(list[-2]) // expect: d
IO.write(list[-1]) // expect: 555
