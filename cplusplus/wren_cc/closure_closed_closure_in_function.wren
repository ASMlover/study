
var f = nil

{
  var local = "local"
  f = fn {
    io.write(local)
  }
}

f.call // expect: local
