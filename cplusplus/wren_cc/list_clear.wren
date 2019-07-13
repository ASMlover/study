
var a = [1, 2, 3]
a.clear
IO.write(a) // expect: []
IO.write(a.len) // expect: 0

IO.write([1, 2].clear) // expect: nil
