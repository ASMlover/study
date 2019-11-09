
var fiber1 = Fiber.new {
  IO.print("1 before")
  Fiber.abort("abort!")
  IO.print("1 after")
}

var fiber2 = Fiber.new {
  IO.print("2 before")
  fiber1.call()
  IO.print("2 after")
}

var fiber3 = Fiber.new {
  IO.print("3 before")
  fiber2.call()
  IO.print("3 after")
}

var fiber4 = Fiber.new {
  IO.print("4 before")
  fiber3.try()
  IO.print("4 after")
}

var fiber5 = Fiber.new {
  IO.print("5 before")
  fiber4.call()
  IO.print("5 after")
}

fiber5.call()
// expect: 5 before
// expect: 4 before
// expect: 3 before
// expect: 2 before
// expect: 1 before
// expect: 4 after
// expect: 5 after
IO.print("after") // expect: after

IO.print(fiber1.error) // expect: abort!
IO.print(fiber2.error) // expect: abort!
IO.print(fiber3.error) // expect: abort!
IO.print(fiber4.error) // expect: nil
IO.print(fiber5.error) // expect: nil
