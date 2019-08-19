
class Base {
  foo { IO.print("Base.foo") }
  foo(a) { IO.print("Base.foo(a)") }
  foo(a, b) { IO.print("Base.foo(a, b)") }
}

class Derived is Base {
  foo(a) {
    IO.print("Derived.foo(a)")
    super
    super(1)
    super(1, 2)
  }
}

(new Derived).foo(1)
// expect: Derived.foo(a)
// expect: Base.foo
// expect: Base.foo(a)
// expect: Base.foo(a, b)