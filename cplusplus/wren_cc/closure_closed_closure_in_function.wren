
var f = nil

{
  var local = "local"
  f = Function.new {
    IO.print(local)
  }
}

f.call() // expect: local
