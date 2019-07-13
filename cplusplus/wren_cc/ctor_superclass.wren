
class A {
  new(arg) {
    IO.write("A.new " + arg)
    _field = arg
  }

  aField { return _field }
}

class B is A {
  new(arg1, arg2) {
    super(arg1)
    IO.write("B.new " + arg2)
    _field = arg2
  }

  bField { return _field }
}

class C is B {
  new {
    super("one", "two")
    IO.write("C.new")
    _field = "c"
  }

  cField { return _field }
}

var c = new C
IO.write(c is A) // expect: true
IO.write(c is B) // expect: true
IO.write(c is C) // expect: true

IO.write(c.aField) // expect: one
IO.write(c.bField) // expect: two
IO.write(c.cField) // expect: c
