
class Outer {
  method {
    IO.print(this) // expect: Outer

    Function.new {
      IO.print(this) // expect: Outer

      class Inner {
        method {
          IO.print(this) // expect: Inner
        }
        toString { "Inner" }
      }

      Inner.ne().method
    }.call()
  }

  toString { "Outer" }
}

Outer.new().method
