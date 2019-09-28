
var a
var b

a = Fiber.new {
  b.call(3) // expect runtime error
}

b = Fiber.new {
  a.call(2)
}

b.call(1)
