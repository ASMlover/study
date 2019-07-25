
class Foo {
  + other { return "Foo " + other }
}

IO.print(new Foo + "value") // expect: Foo value
