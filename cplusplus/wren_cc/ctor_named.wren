
class Foo {
  this named() { _field = "named" }
  this other() { _field = "other" }

  toString { _field }
}

IO.print(Foo.named()) // expect: named
IO.print(Foo.other()) // expect: other

var foo = Foo.named()
IO.print(foo is Foo) // expect: true
IO.print(foo.toString) // expect: named
