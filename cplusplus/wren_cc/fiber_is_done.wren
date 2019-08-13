
var fiber = new Fiber {
  IO.print("111")
  Fiber.yield
  IO.print("222")
}

IO.print(fiber.isDone) // expect: false
fiber.run // expect: 111
IO.print(fiber.isDone) // expect: false
fiber.run // expect: 222
IO.print(fiber.isDone) // expect: true
