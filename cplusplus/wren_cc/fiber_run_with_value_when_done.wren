
var fiber = new Fiber {
  IO.print("run")
}

fiber.run(1) // expect: run
fiber.run(2) // expect runtime error
