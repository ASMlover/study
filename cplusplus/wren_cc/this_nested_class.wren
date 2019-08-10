
class Outer {
  method {
    IO.print(this) // expect: Outer

    new Function {
      IO.print(this) // expect: Outer

      class Inner {
        method {
          IO.print(this) // expect: Inner
        }
        toString { "Inner" }
      }

      (new Inner).method
    }.call
  }

  toString { "Outer" }
}

(new Outer).method
