
var fiber

fiber = Fiber.new {
  fiber.call(2) // expect runtime error
}

fiber.call(1)
