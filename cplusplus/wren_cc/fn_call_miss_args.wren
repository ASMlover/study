
var f = Function.new {|a, b| IO.print(a + b) }
f.call("a") // expect runtime error
