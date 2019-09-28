
class Foo {
  getClosure { Function.new { Function.new { Function.new { toString } } } }
  toString { "Foo" }
}

var closure = Foo.new().getClosure
IO.print(closure.call().call().call()) // expect: Foo
