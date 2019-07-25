
var f = nil

class Foo {
  method(param) {
    f = fn {
      IO.print(param)
    }
  }
}

(new Foo).method("parameter")
f.call // expect: parameter
