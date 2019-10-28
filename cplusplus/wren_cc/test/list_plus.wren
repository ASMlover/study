
IO.print([1, 2, 3] + [4, 5, 6]) // expect: [1, 2, 3, 4, 5, 6]
IO.print([1, 2, 3] + (4..6)) // expect: [1, 2, 3, 4, 5, 6]
IO.print([1, 2, 3] + "abc") // expect: [1, 2, 3, a, b, c]
IO.print([] + []) // expect: []
IO.print([1, 2] + []) // expect: [1, 2]
IO.print([] + [3, 4]) // expect: [3, 4]

// does not modify original list
var a = [1, 2, 3]
IO.print(a * 5)
IO.print(a) // expect: [1, 2, 3]
