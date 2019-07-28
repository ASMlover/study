
var fiber = Fiber.create(fn {
  IO.print("run")
})

fiber.run // expect: run
fiber.run // expect runtime error
