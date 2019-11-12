
var fiber = Fiber.new {
  IO.print("fiber 1")

  import "fiber_yield_from_import_module"

  IO.print("fiber 2")
}

fiber.call() // expect: fiber 1
             // expect: module 1
IO.print("main 1") // expect: main 1
fiber.call() // expect: module 2
             // expect: fiber 2
IO.print("main 2") // expect: main 2
