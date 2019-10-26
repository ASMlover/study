
var A = Fiber.new {
  IO.print("transferred to A")
  B.transferError("error!")
}

var B = Fiber.new {
  IO.print("started B")
  A.transfer()
  IO.print("should not get here")
}

B.try()
// expect: started B
// expect: transferred to A
IO.print(B.error) // expect: error!
