
var f = nil
var g = nil

{
  var local = "local"
  f = Function.new {
    IO.print(local)
    local = "after f"
    IO.print(local)
  }

  g = Function.new {
    IO.print(local)
    local = "after g"
    IO.print(local)
  }
}

f.call()
// expect: local
// expect: after f

g.call()
// expect: after f
// expect: after g
