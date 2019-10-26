
var main = Fiber.current

var fiber = Fiber.new {
  IO.print("fiber")
  IO.print(main.transfer())
}

fiber.transfer() // expect: fiber
IO.print("main") // expect: main
fiber.transfer("transfer") // expect: transfer

IO.print("nope")
