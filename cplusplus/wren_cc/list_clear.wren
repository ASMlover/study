
var a = [1, 2, 3]
a.clear
io.write(a) // expect: []
io.write(a.len) // expect: 0

io.write([1, 2].clear) // expect: nil
