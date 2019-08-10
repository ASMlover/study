
var global = "global"

new Function {
  IO.print(global) // expect: global
}.call
