
var b = Fiber.create {
  IO.print("fiber b")
}

var a = Fiber.create {
  IO.print("begin fiber a")
  b.run
  IO.print("end fiber a")
}

IO.print("begin man")
a.run
IO.print("end main")

// expect: begin main
// expect: begin fiber a
// expect: fiber b
// expect: end fiber a
// expect: end main
