
IO.print(Numeric is Class) // expect: true
IO.print(true is Bool) // expect: true
IO.print(new Function { 1 } is Function) // expect: true
IO.print(123 is Numeric) // expect: true
IO.print(nil is Nil) // expect: true
IO.print("s" is String) // expect: true

IO.print(Numeric is Bool) // expect: false
IO.print(nil is Class) // expect: false
IO.print(true is Function) // expect: false
IO.print(new Function { 1 } is Numeric) // expect: false
IO.print("s" is Nil) // expect: false
IO.print(123 is String) // expect: false

IO.print(Numeric is Object) // expect: true
IO.print(nil is Object) // expect: true
IO.print(true is Object) // expect: true
IO.print(new Function { 1 } is Object) // expect: true
IO.print("s" is Object) // expect: true
IO.print(123 is Object) // expect: true

// inheritance
class A {}
class B is A {}
class C is B {}
var a = new A
var b = new B
var c = new C

IO.print(a is A) // expect: true
IO.print(a is B) // expect: false
IO.print(a is C) // expect: false
IO.print(b is A) // expect: true
IO.print(b is B) // expect: true
IO.print(b is C) // expect: false
IO.print(c is A) // expect: true
IO.print(c is B) // expect: true
IO.print(c is C) // expect: true

// ignore newline after `is`
IO.print(123 is
  Numeric) // expect: trur
