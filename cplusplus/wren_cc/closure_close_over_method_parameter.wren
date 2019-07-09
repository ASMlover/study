
var f = nil

class Foo {
  method(param) {
    f = fn {
      io.write(param)
    }
  }
}

(new Foo).method("parameter")
f.call // expect: parameter
