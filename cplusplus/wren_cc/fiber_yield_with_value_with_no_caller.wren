
var a = new Fiber {
  IO.print("before")
  Fiber.yield(1)
}

var b = new Fiber { a.run() }
var c = new Fiber { b.run() }
c.run()
IO.print("not reached")
