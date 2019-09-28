
var F = nil

class Foo {
  method(param) {
    F = Function.new {
      IO.print(param)
    }
  }
}

Foo.new().method("parameter")
F.call() // expect: parameter
