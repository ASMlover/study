
var f = nil

{
  var local = "local"
  f = new Function {
    IO.print(local)
  }
}

f.call // expect: local
