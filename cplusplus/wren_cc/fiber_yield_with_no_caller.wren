
var a = new Fiber {
  IO.print("before") // expect: before
  Fiber.yield()
}

var b = new Fiber { a.run() }
var c = new Fiber { b.run() }
c.run()
IO.print("not reached")
