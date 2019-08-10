
var f = new Function {
  if (true) { return }
  IO.print("bad")
}

IO.print(f.call) // expect: nil
