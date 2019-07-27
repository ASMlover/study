
var fiber = Fiber.create(fn {
  IO.print("fiber")
  return "result"
})

var result = fiber.run // expect: fiber
IO.print(result) // expect: result
