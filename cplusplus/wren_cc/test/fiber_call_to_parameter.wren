
var fiber = Fiber.new {|value|
  IO.print(value)
}

IO.print("before") // expect: before
fiber.call() // expect: nil
IO.print("after") // expect: after
