
var map = {
  "a": 1,
  "b": 2,
}

IO.print(map["a"]) // expect: 1
IO.print(map["b"]) // expect: 2

// invalid syntax
IO.print(Fiber.new { Meta.eval("{,}") }.try()) // expect: could not compile source code
IO.print(Fiber.new { Meta.eval("{1:1,,}") }.try()) // expect: could not compile source code
IO.print(Fiber.new { Meta.eval("{1:1,,2:2}") }.try()) // expect: could not compile source code
