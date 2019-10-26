
var fiber = Fiber.new {
  Fiber.abort(nil)
  IO.print("get here") // expect: get here
  Fiber.yield("value")
}

IO.print(fiber.try()) // expect: value
IO.print(fiber.isDone) // expect: false
IO.print(fiber.error) // expect: nil
