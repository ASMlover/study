
class A {
  this new(arg) {
    io.write("A.new " + arg)
  }
}

class B is A {
  this otherName(arg1, arg2) super.new(arg1) {
    io.write("B.otherName " + arg2)
  }
}

class C is B {
  this create super.otherName("one", "two") {
    io.write("C.create")
  }
}

var c = C.create
io.write(c is A) // expect: true
io.write(c is B) // expect: true
io.write(c is C) // expect: true
