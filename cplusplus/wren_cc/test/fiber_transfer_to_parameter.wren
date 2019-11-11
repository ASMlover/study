
var a = Fiber.new {|param|
  IO.print("a " + param.toString)
}

var b = Fiber.new {|param|
  IO.print("b before " + param.toString)
  a.transfer()
  IO.print("b after")
}

var c = Fiber.new {|param|
  IO.print("c before " + param.toString)
  b.transfer()
  IO.print("c after")
}

IO.print("start") // expect: start
c.transfer()
// expect: c before nil
// expect: b before nil
// expect: a nil
