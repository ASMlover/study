
var fiber = Fiber.new {
  Fiber.abort(123)
}

IO.print(fiber.try()) // expect: 123
IO.print(fiber.isDone) // expect: true
IO.print(fiber.error) // expect: 123
