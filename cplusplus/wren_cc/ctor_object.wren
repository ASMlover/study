
class Foo {
  construct new() {
    super() // should not cause a no method error
    IO.print("ok")
  }
}

Foo.new() // expect: ok
