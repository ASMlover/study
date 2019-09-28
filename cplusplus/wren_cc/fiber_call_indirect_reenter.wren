
var a
var b

a = Fiber.new {
  b.call() // expect runtime error
}

b = Fiber.new {
  a.call()
}

b.call()
