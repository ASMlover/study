
class Foo {
  getClosure { new Function { toString } }
  toString { "Foo" }
}

var closure = (new Foo).getClosure
IO.print(closure.call) // expect: Foo
