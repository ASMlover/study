
var a = Fiber.new {
  IO.print("before")
  Fiber.yield(1)
IO.print("not reached")
}

var b = Fiber.new { a.transfer() }
var c = Fiber.new { b.transfer() }
c.transfer()
IO.print("not reached")
