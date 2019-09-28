
var a = Fiber.new {
  IO.print("before")
  Fiber.yield(1)
}

var b = Fiber.new { a.run() }
var c = Fiber.new { b.run() }
c.run()
IO.print("not reached")
