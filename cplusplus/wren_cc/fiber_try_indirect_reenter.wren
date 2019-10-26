
var a
var b

a = Fiber.new {
  b.try() // expect runtime error
}

b = Fiber.new {
  a.call()
}

b.call()
