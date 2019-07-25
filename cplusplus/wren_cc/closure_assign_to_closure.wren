
var f = nil
var g = nil

{
  var local = "local"
  f = fn {
    IO.print(local)
    local = "after f"
    IO.print(local)
  }

  g = fn {
    IO.print(local)
    local = "after g"
    IO.print(local)
  }
}

f.call
// expect: local
// expect: after f

g.call
// expect: after f
// expect: after g
