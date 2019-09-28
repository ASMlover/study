
var fiber = Fiber.new {
  IO.print("fiber")
}

IO.print("before") // expect: before
IO.print(fiber.try())
// expect: fiber
// expect: nil
IO.print("after") // expect: after
