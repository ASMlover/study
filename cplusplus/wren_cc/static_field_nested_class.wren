
class Outer {
  construct new() {}

  static staticMethod {
    __field = "outer"
    IO.print(__field) // expect: outer

    class Inner {
      construct new() {}

      static staticMethod {
        __field = "inner"
        IO.print(__field) // expect: inner
      }
    }

    Inner.staticMethod
    IO.print(__field) // expect: outer
  }

  instanceMethod {
    __field = "outer"
    IO.print(__field) // expect: outer

    class Inner {
      construct new() {}

      instanceMethod {
        __field = "inner"
        IO.print(__field) // expect: inner
      }
    }

    Inner.new().instanceMethod
    IO.print(__field) // expect: outer
  }
}

Outer.staticMethod
Outer.new().instanceMethod
