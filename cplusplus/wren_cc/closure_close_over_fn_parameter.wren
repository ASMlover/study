
var f = nil

Function.new {|param|
  f = Function.new {
    IO.print(param)
  }
}.call("parameter")

f.call() // expect: parameter
