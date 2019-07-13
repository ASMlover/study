
class Base {
  foo {
    IO.write("Base.foo")
  }
}

class Derived is Base {
  bar {
    IO.write("Derived.bar")
    super.foo
  }
}

(new Derived).bar
// expect: Derived.bar
// expect: Base.foo
