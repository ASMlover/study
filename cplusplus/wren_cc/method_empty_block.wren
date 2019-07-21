
class Foo {
  bar {}
}

IO.write((new Foo).bar) // expect: nil
