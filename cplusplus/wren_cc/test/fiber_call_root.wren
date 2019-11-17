
var root = Fiber.current
IO.print("begin root") // expect: begin root

Fiber.new {
  IO.print("in new fiber") // expect: in new fiber
  root.call() // expect runtime error
  IO.print("called root")
}.transfer()
