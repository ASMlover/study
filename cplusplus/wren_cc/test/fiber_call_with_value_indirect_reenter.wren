
var A = Fiber.new {
  B.call(3) // expect runtime error
}

var B = Fiber.new {
  A.call(2)
}

B.call(1)
