
var a
var b

a = new Fiber {
  b.call(3) // expect runtime error
}

b = new Fiber {
  a.call(2)
}

b.call(1)
