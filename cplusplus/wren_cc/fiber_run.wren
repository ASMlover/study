
var fiber = Fiber.create {
  IO.print("fiber")
}

IO.print("before") // expect: before
fiber.run // expect: fiber
IO.print("after") // expect: after
