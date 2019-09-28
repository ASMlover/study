
var a = Fiber.new {
  IO.print("before") // expect: before
  Fiber.yield()
}

var b = Fiber.new { a.run() }
var c = Fiber.new { b.run() }
c.run()
IO.print("not reached")
