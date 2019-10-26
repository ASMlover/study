
class Foo {
  construct new() {}

  getter {
    IO.print("getter")
  }

  setter=(value) {
    IO.print("setter")
  }

  method(a) {
    IO.print("method")
  }

  test {
    getter // expect: getter
    setter = "value" // expect: setter
    method("arg") // expect: method
  }
}

Foo.new().test
