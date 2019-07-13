
var f = nil

class Foo {
  method(param) {
    f = fn {
      IO.write(param)
    }
  }
}

(new Foo).method("parameter")
f.call // expect: parameter
