
var fiber

fiber = new Fiber {
  fiber.call(2) // expect runtime error
}

fiber.call(1)
