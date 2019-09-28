
class Foo {
  +(other) { "Foo " + other }
}

IO.print(Foo.new() + "value") // expect: Foo value
