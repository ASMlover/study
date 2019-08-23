
var F = nil

class Foo {
  method(param) {
    F = new Function {
      IO.print(param)
    }
  }
}

(new Foo).method("parameter")
F.call // expect: parameter
