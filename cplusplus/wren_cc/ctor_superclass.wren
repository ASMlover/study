
class A {
  new(arg) {
    io.write("A.new " + arg)
    _field = arg
  }

  aField { return _field }
}

class B is A {
  new(arg1, arg2) {
    super(arg1)
    io.write("B.new " + arg2)
    _field = arg2
  }

  bField { return _field }
}

class C is B {
  new {
    super("one", "two")
    io.write("C.new")
    _field = "c"
  }

  cField { return _field }
}

var c = new C
io.write(c is A) // expect: true
io.write(c is B) // expect: true
io.write(c is C) // expect: true

io.write(c.aField) // expect: one
io.write(c.bField) // expect: two
io.write(c.cField) // expect: c
