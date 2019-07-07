
class Base {
  foo {
    io.write("Base.foo")
  }
}

class Derived is Base {
  bar {
    io.write("Derived.bar")
    super.foo
  }
}

Derived.new.bar
// expect: Derived.bar
// expect: Base.foo
