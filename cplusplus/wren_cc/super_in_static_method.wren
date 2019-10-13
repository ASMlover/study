
class Foo {
  static name {
    IO.print("Foo.name") // expect: Foo.name
    IO.print(super) // expect: Foo
  }
}

Foo.name
