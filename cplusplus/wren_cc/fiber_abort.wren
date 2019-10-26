
var fiber = Fiber.new {
  Fiber.abort("error message")
}

IO.print(fiber.try()) // expect: error message
IO.print(fiber.isDone) // expect: true
IO.print(fiber.error) // expect: error message
