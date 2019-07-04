
var f = nil
var g = nil

{
  var local = "local"
  f = fn {
    io.write(local)
    local = "after f"
    io.write(local)
  }

  g = fn {
    io.write(local)
    local = "after g"
    io.write(local)
  }
}

f.call
// expect: local
// expect: after f

g.call
// expect: after f
// expect: after g
