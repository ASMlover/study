
IO.print([1, 2, 3] * 0) // expect: []
IO.print([1, 2, 3] * 1) // expect: [1, 2, 3]
IO.print([1, 2, 3] * 4) // expect: [1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3]

// does not modify original list
var a = [1, 2, 3]
a * 5
IO.print(a) // expect: [1, 2, 3]
