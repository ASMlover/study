
io.write(Numeric is Class) // expect: true
io.write(true is Bool) // expect: true
io.write((fn 1) is Function) // expect: true
io.write(123 is Numeric) // expect: true
io.write(nil is Nil) // expect: true
io.write("s" is String) // expect: true

io.write(Numeric is Bool) // expect: false
io.write(nil is Class) // expect: false
io.write(true is Function) // expect: false
io.write((fn 1) is Numeric) // expect: false
io.write("s" is Nil) // expect: false
io.write(123 is String) // expect: false

io.write(Numeric is Object) // expect: true
io.write(nil is Object) // expect: true
io.write(true is Object) // expect: true
io.write((fn 1) is Object) // expect: true
io.write("s" is Object) // expect: true
io.write(123 is Object) // expect: true

// inheritance
class A {}
class B is A {}
class C is B {}
var a = new A
var b = new B
var c = new C

io.write(a is A) // expect: true
io.write(a is B) // expect: false
io.write(a is C) // expect: false
io.write(b is A) // expect: true
io.write(b is B) // expect: true
io.write(b is C) // expect: false
io.write(c is A) // expect: true
io.write(c is B) // expect: true
io.write(c is C) // expect: true
