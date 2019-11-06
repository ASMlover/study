
var list = List.new(5)

IO.print(list.count) // expect: 5
IO.print(list) // expect: [nil, nil, nil, nil, nil]

var list2 = List.new(5, 2)
IO.print(list2.count) // expect: 5
IO.print(list2) // expect: [2, 2, 2, 2, 2]
