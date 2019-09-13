
var a
var b

a = new Fiber {
  b.try() // expect runtime error
}

b = new Fiber {
  a.call()
}

b.call()
