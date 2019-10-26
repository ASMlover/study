
var fiber = Fiber.new {
  IO.print("111")
  Fiber.yield()
  IO.print("222")
}

IO.print(fiber.isDone) // expect: false
fiber.call() // expect: 111
IO.print(fiber.isDone) // expect: false
fiber.call() // expect: 222
IO.print(fiber.isDone) // expect: true
