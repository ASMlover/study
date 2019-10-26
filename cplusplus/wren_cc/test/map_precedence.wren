
var name = "value"

var map = {
  // primary
  name: name,
  1: true,

  // call
  name.count: name.count,
  name[0]: name[1],

  // unary
  -1: -2,
  ~3: !false,

  // allow any expression for a value
  "key": true ? 1 : 2
}

IO.print(map[name]) // expect: value
IO.print(map[1]) // expect: true
IO.print(map[name.count]) // expect: 5
IO.print(map[name[0]]) // expect: a
IO.print(map[-1]) // expect: -2
IO.print(map[~3]) // expect: true
IO.print(map["key"]) // expect: 1
