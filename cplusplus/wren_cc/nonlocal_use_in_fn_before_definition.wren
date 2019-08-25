
var f = new Function {
  IO.print(Global)
}

var Global = "global"
f.call // expect: global
