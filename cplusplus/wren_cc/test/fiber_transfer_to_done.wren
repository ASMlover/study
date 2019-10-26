
var a = Fiber.new {
  IO.print("run")
}

a.call() // expect: run
a.transfer() // expect runtime error
