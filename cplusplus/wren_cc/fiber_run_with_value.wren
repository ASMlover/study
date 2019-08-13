
var fiber = new Fiber {
  IO.print("fiber")
}

IO.print("before") // expect: before
fiber.run("ignored") // expect: fiber
IO.print("after") // expect: after
