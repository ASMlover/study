
class A {
  construct new(arg) {
    IO.print("A.new ", arg)
    _field = arg
  }

  aField { _field }
}

class B is A {
  construct new(arg1, arg2) {
    super(arg1)
    IO.print("B.new ", arg2)
    _field = arg2
  }

  bField { _field }
}

class C is B {
  construct new() {
    super("one", "two")
    IO.print("C.new")
    _field = "c"
  }

  cField { _field }
}

var c = C.new()
IO.print(c is A) // expect: true
IO.print(c is B) // expect: true
IO.print(c is C) // expect: true

IO.print(c.aField) // expect: one
IO.print(c.bField) // expect: two
IO.print(c.cField) // expect: c
