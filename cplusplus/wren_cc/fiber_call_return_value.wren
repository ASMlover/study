
var fiber = Fiber.new {
  IO.print("fiber") // expect:  fiber
  return "result"
}

IO.print(fiber.call()) // expect: result
