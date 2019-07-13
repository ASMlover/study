
var f = nil

fn (param) {
  f = fn {
    IO.write(param)
  }
}.call("parameter")

f.call // expect: parameter
