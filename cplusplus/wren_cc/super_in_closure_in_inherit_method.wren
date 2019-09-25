
class A {
  callSuperToString {
    return new Function { super.toString }.call()
  }

  toString { "A.toString" }
}

class B is A {}

IO.print((new B).callSuperToString) // expect: A.toString
