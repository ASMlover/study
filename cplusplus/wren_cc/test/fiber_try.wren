
var fiber = Fiber.new {
  IO.print("before")
  true.unknownMethod
  IO.print("after")
}
IO.print(fiber.try())
// expect: before
// expect: `Bool` does not implement `unknownMethod`
IO.print("after try") // expect: after

var fiber2 = Fiber.new {
  var fiberInner = Fiber.new {
    IO.print("before")
    true.unknownMethod
    IO.print("after")
  }
  fiberInner.call()
}
IO.print(fiber2.try())
// expect: before
// expect: `Bool` does not implement `unknownMethod`
IO.print("after try") // expect: after try

var fiber3 = Fiber.new {
  var fiberInner = Fiber.new {
    var fiberInnerInner = Fiber.new {
      IO.print("before")
      true.unknownMethod
      IO.print("after")
    }
    fiberInnerInner.call()
  }
  fiberInner.call()
}
IO.print(fiber3.try())
// expect: before
// expect: `Bool` does not implement `unknownMethod`
IO.print("after try") // expect: after try
