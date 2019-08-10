
class Foo {
  new { IO.print("zero") }
  new(a) { IO.print(a) }
  new(a, b) { IO.print(a + b) }

  toString { "Foo" }
}

// can overload by arity
new Foo // expect: zero
new Foo("one") // expect: one
new Foo("one", "two") // expect: onetwo

// returns the new instance
var foo = new Foo // expect: zero
IO.print(foo is Foo) // expect: true
IO.print(foo.toString) // expect: Foo
