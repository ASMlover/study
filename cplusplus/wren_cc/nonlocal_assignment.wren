
var NonLocal = "before"
IO.print(NonLocal) // expect: before
NonLocal = "after"
IO.print(NonLocal) // expect: after

class Foo {
  static method {
    NonLocal = "method"
  }
}

Foo.method
IO.print(NonLocal) // expect: method

Function.new {
  NonLocal = "fn"
}.call()
IO.print(NonLocal) // expect: fn
