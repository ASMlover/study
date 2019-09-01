
var m = {}
IO.print(m.len) // expect: 0
m["one"] = "value"
IO.print(m.len) // expect: 1
m["two"] = "value"
IO.print(m.len) // expect: 2
m["three"] = "value"
IO.print(m.len) // expect: 3

// adding existing does not increase count
m["two"] = "new value"
IO.print(m.len) // expect: 3
