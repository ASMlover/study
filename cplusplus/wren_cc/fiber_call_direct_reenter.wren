
var fiber

fiber = Fiber.new {
  fiber.call() // expect runtime error
}

fiber.call()
