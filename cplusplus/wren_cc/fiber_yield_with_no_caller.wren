
var a = new Fiber {
  Fiber.yield() // expect runtime error
}

var b = new Fiber { a.run() }
var c = new Fiber { b.run() }
c.run()
