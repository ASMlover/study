
var f = nil

{
  var local = "local"
  f = fn {
    IO.print(local)
  }
}

f.call // expect: local
