
var fiber = Fiber.new {
  Fiber.abort("Error!!!")
  IO.print("should not get here")
}

fiber.try()
fiber.call() // expect runtime error