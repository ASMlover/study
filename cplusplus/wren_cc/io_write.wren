
class Foo {
  toString { return "Foo.toString" }
}

// calls toString on argument
IO.print(new Foo) // expect: Foo.toString

// returns argument
var result = IO.print(123) // expect: 123
IO.print(result == 123) // expect: true
