
var f = nil

fn (param) {
  f = fn {
    io.write(param)
  }
}.call("parameter")

f.call // expect: parameter
