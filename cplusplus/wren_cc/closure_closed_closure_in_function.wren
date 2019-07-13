
var f = nil

{
  var local = "local"
  f = fn {
    IO.write(local)
  }
}

f.call // expect: local
