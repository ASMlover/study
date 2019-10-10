
var F = nil

class Foo {
  construct new() {}

  method(param) {
    F = Function.new {
      IO.print(param)
    }
  }
}

Foo.new().method("parameter")
F.call() // expect: parameter
