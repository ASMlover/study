
var fiber = new Fiber {
  IO.print("fiber")
}

IO.print("before") // expect: before
fiber.run // expect: fiber

// this does not get run since we exit when the fiber completes
IO.print("nope")
