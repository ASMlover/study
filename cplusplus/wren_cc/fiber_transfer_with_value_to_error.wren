
var a = Fiber.new {
  Fiber.abort("error!")
  IO.print("should not get here")
}

a.try()
a.transfer("blah") // expect runtime error
