
var f = nil
var g = nil

{
  var local = "local"
  f = fn {
    IO.write(local)
    local = "after f"
    IO.write(local)
  }

  g = fn {
    IO.write(local)
    local = "after g"
    IO.write(local)
  }
}

f.call
// expect: local
// expect: after f

g.call
// expect: after f
// expect: after g
