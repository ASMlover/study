
var f = Function.new {
  return
  IO.print("bad")
}

IO.print(f.call()) // expect: nil
