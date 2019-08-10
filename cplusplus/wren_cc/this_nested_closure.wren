
class Foo {
  getClosure { new Function { new Function { new Function { toString } } } }
  toString { return "Foo" }
}

var closure = (new Foo).getClosure
IO.print(closure.call.call.call) // expect: Foo
