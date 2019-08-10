
var f = nil

class Foo {
  method(param) {
    f = new Function {
      IO.print(param)
    }
  }
}

(new Foo).method("parameter")
f.call // expect: parameter
