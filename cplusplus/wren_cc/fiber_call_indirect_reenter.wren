
var a
var b

a = new Fiber {
  b.call // expect runtime error
}

b = new Fiber {
  a.call
}

b.call
