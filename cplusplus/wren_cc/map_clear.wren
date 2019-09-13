
var a = {1:1, 2:2, 3:3}
a.clear()
IO.print(a) // expect: {}
IO.print(a.len) // expect: 0

// return nil
IO.print({1: 2}.clear()) // expect: nil
