
var list = List.filled(3, "value")
IO.print(list.count) // expect: 3
IO.print(list) // expect: [value, value, value]

list = List.filled(0, "value")
IO.print(list.count) // expect: 0
IO.print(list) // expect: []
