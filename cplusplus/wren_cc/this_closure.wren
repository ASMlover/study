
class Foo {
  getClosure { Function.new { toString } }
  toString { "Foo" }
}

var closure = Foo.new().getClosure
IO.print(closure.call()) // expect: Foo
