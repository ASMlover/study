
var a = new Fiber {
  Fiber.yield(1) // expect runtime error
}

var b = new Fiber { a.run() }
var c = new Fiber { b.run() }
c.run()
