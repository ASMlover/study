
var a = new Fiber {
  IO.print("run")
}

// run a through an intermediate fiber since it will get discarded and we
// need to return to the main one after a completes
var b = new Fiber {
  a.run
  IO.print("nope")
}

b.call // expect: run
a.run // expect runtime error
