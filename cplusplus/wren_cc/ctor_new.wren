
class Foo {
  this new { io.write("zero") }
  this new(a) { io.write(a) }
  this new(a, b) { io.write(a + b) }

  toString { return "Foo" }
}

// can overload by arity
Foo.new // expect: zero
Foo.new("one") // expect: one
Foo.new("one", "two") // expect: onetwo

// returns the new instance
var foo = Foo.new // expect: zero
io.write(foo is Foo) // expect: true
io.write(foo.toString) // expect: Foo
