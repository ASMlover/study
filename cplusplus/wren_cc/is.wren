
IO.write(Numeric is Class) // expect: true
IO.write(true is Bool) // expect: true
IO.write((fn 1) is FunctIO.) // expect: true
IO.write(123 is Numeric) // expect: true
IO.write(nil is Nil) // expect: true
IO.write("s" is String) // expect: true

IO.write(Numeric is Bool) // expect: false
IO.write(nil is Class) // expect: false
IO.write(true is FunctIO.) // expect: false
IO.write((fn 1) is Numeric) // expect: false
IO.write("s" is Nil) // expect: false
IO.write(123 is String) // expect: false

IO.write(Numeric is Object) // expect: true
IO.write(nil is Object) // expect: true
IO.write(true is Object) // expect: true
IO.write((fn 1) is Object) // expect: true
IO.write("s" is Object) // expect: true
IO.write(123 is Object) // expect: true

// inheritance
class A {}
class B is A {}
class C is B {}
var a = new A
var b = new B
var c = new C

IO.write(a is A) // expect: true
IO.write(a is B) // expect: false
IO.write(a is C) // expect: false
IO.write(b is A) // expect: true
IO.write(b is B) // expect: true
IO.write(b is C) // expect: false
IO.write(c is A) // expect: true
IO.write(c is B) // expect: true
IO.write(c is C) // expect: true
