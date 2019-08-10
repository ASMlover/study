
var f = new Function {|a, b| IO.print(a + b) }
f.call("a") // expect runtime error
