
var fiber = Fiber.new {
  IO.print("fiber 1")
  IO.print(Fiber.yield())
  IO.print(Fiber.yield())
}

fiber.call() // expect: fiber 1
IO.print("main 1") // expect: main 1
fiber.call("call 1") // expect: call 1
IO.print("main 2") // expect: main 2
fiber.call() // expect: nil
IO.print("main 3") // expect: main 3
