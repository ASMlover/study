
class Base {}

class Derived is Base {
  construct new() {}
  foo { super.doesNotExist } // expect runtime error
}

Derived.new().foo
