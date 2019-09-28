
var fiber = Fiber.new {
  IO.print("fiber 1")
  Fiber.yield("yield 1")
  IO.print("fiber 2")
  Fiber.yield("yield 2")
  IO.print("fiber 3")
}

var result = fiber.call() // expect: fiber 1
IO.print(result) // expect: yield 1
result = fiber.call() // expect: fiber 2
IO.print(result) // expect: yield 2
result = fiber.call() // expect: fiber 3
IO.print(result) // expect: nil
