
var f = new Function {
  return "ok"
  IO.print("bad")
}

IO.print(f.call()) // expect: ok
