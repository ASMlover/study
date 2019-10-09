
// allow after `[` and `,` and before `]`
var list = [

  "a",
  "b"

]

IO.print(list[0]) // expect: a
IO.print(list[1]) // expect: b

// newline after trailing comma
list = ["c",

]
IO.print(list[0]) // expect: c

// newline in empty list
list = [

]
IO.print(list.len) // expect: 0
