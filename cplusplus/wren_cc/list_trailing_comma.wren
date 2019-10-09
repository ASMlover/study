
var list = [
  "a",
  "b",
]

IO.print(list[0]) // expect: a
IO.print(list[1]) // expect: b

// invalid syntax
IO.print(Fiber.new { Meta.eval("[,]") }.try()) // expect: could not compile source code
IO.print(Fiber.new { Meta.eval("[1,,]") }.try()) // expect: could not compile source code
IO.print(Fiber.new { Meta.eval("[1,,2]") }.try()) // expect: could not compile source code
