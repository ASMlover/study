
class Outer {
  construct new() {}

  getter {
    IO.print("outer getter")
  }

  setter=(value) {
    IO.print("outer setter")
  }

  method(a) {
    IO.print("outer method")
  }

  test {
    getter // expect: outer getter
    setter = "value" // expect: outer setter
    method("arg") // expect: outer method

    class Inner {
      construct new() {}

      getter {
        IO.print("inner getter")
      }

      setter=(value) {
        IO.print("inner setter")
      }

      method(a) {
        IO.print("inner method")
      }

      test {
        getter // expect: inner getter
        setter = "value" // expect: inner setter
        method("arg") // expect: inner method
      }
    }

    Inner.new().test

    getter // expect: outer getter
    setter = "value" // expect: outer setter
    method("arg") // expect: outer method
  }
}

Outer.new().test
