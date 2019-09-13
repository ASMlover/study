
var f = nil

new Function {|param|
  f = new Function {
    IO.print(param)
  }
}.call("parameter")

f.call() // expect: parameter
