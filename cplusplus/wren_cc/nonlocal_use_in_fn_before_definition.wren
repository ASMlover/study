
var f = Function.new {
  IO.print(Global)
}

var Global = "global"
f.call() // expect: global
