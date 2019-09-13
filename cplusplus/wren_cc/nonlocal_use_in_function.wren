
var Global = "global"

new Function {
  IO.print(Global) // expect: global
}.call()
