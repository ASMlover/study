
class A {
  this new(arg) {
    io.write("A.new " + arg)
    _field = arg
  }

  aField { return _field }
}

class B is A {
  this otherName(arg1, arg2) super.new(arg1) {
    io.write("B.otherName " + arg2)
    _field = arg2
  }

  bField { return _field }
}

class C is B {
  this create super.otherName("one", "two") {
    io.write("C.create")
    _field = "c"
  }

  cField { return _field }
}

var c = C.create
io.write(c is A) // expect: true
io.write(c is B) // expect: true
io.write(c is C) // expect: true

io.write(c.aField) // expect: one
io.write(c.bField) // expect: two
io.write(c.cField) // expect: c
