
var list = ["a", "b", "c", "d", 555]
io.write(list[0]) // expect: a
io.write(list[1]) // expect: b
io.write(list[2]) // expect: c
io.write(list[3]) // expect: d
io.write(list[4]) // expect: 555

io.write(list[-5]) // expect: a
io.write(list[-4]) // expect: b
io.write(list[-3]) // expect: c
io.write(list[-2]) // expect: d
io.write(list[-1]) // expect: 555

io.write(list[5]) // expect: nil
io.write(list[-6]) // expect: nil

io.write(list[true]) // expect: nil
io.write(list[1.5]) // expect: nil
io.write(list["1"]) // expect: nil
