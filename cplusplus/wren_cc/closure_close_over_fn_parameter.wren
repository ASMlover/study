
var f = nil

fn (param) {
  f = fn {
    IO.print(param)
  }
}.call("parameter")

f.call // expect: parameter
