
var a = [1, 2, 3]
a.clear
IO.print(a) // expect: []
IO.print(a.len) // expect: 0

IO.print([1, 2].clear) // expect: nil
