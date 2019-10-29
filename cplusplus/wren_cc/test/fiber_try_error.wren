
var fiber = Fiber.new {
  IO.print("try")
  Fiber.abort("error")
}

fiber.try() // expect: try
fiber.try() // expect runtime error
