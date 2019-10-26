
var a = Fiber.new {
  IO.print("before") // expect: before
  Fiber.yield()
  IO.print("not reached")
}

var b = Fiber.new { a.transfer() }
var c = Fiber.new { b.transfer() }
c.transfer()
IO.print("not reached")
