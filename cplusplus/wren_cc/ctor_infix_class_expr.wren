
class Foo {
  + other { return "Foo " + other }
}

IO.write(new Foo + "value") // expect: Foo value
