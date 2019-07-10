
class Foo {
  + other { return "Foo " + other }
}

io.write(new Foo + "value") // expect: Foo value
