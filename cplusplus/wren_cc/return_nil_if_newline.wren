
var f = new Function {
  return
  IO.print("bad")
}

IO.print(f.call) // expect: nil
