
class Foo {
  toString { return "Foo.toString" }
}

// calls toString on argument
IO.write(new Foo) // expect: Foo.toString

// returns argument
var result = IO.write(123) // expect: 123
IO.write(result == 123) // expect: true
