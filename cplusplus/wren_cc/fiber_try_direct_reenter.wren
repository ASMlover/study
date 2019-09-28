
 var fiber

 fiber = Fiber.new {
  fiber.try() // expect runtime error
 }

 fiber.call()
