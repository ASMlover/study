
var fiber = Fiber.new {}
var map = {
  nil: "nil value",
  true: "true value",
  false: "false value",
  0: "zero",
  1.2: "1 point 2",
  List: "list class",
  "nil": "string value",
  (1..3): "1 to 3",
  fiber: "fiber"
}

IO.print(map[nil]) // expect: nil value
IO.print(map[true]) // expect: true value
IO.print(map[false]) // expect: false value
IO.print(map[0]) // expect: zero
IO.print(map[1.2]) // expect: 1 point 2
IO.print(map[List]) // expect: list class
IO.print(map["nil"]) // expect: string value
IO.print(map[1..3]) // expect: 1 to 3
IO.print(map[fiber]) // expect: fiber

IO.print(map.len) // expect: 9

map[nil] = "new nil value"
map[!false] = "new true value"
map[!true] = "new false value"
map[2 - 2] = "new zero"
map[1.2] = "new 1 point 2"
map[[].type] = "new list class"
map["ni" + "l"] = "new string value"
map[(3 - 2)..(1 + 2)] = "new 1 to 3"

IO.print(map[nil]) // expect: new nil value
IO.print(map[true]) // expect: new true value
IO.print(map[false]) // expect: new false value
IO.print(map[0]) // expect: new zero
IO.print(map[1.2]) // expect: new 1 point 2
IO.print(map[List]) // expect: new list class
IO.print(map["nil"]) // expect: new string value
IO.print(map[1..3]) // expect: new 1 to 3

IO.print(map.len) // expect: 9
