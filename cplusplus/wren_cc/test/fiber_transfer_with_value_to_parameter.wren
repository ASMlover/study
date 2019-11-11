
var a = Fiber.new {|param|
  IO.print("a " + param.toString)
}

var b = Fiber.new {|param|
  IO.print("b before " + param.toString)
  a.transfer("from b")
  IO.print("b after")
}

var c = Fiber.new {|param|
  IO.print("c before " + param.toString)
  b.transfer("from c")
  IO.print("c after")
}

IO.print("start") // expect: start
c.transfer("from main")
// expect: c before from main
// expect: b before from c
// expect: a from b
