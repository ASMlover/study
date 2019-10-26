
var A = Fiber.new {
  B.transferError(123)
}

var B = Fiber.new {
  A.transfer()
}

B.try()
IO.print(B.error) // expect: 123
