
var fiber = new Fiber {
  IO.print("call")
}

fiber.call(1) // expect: call
fiber.call(2) // expect runtime error
