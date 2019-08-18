
class Base {}

class Derived is Base {
  foo { super.doesNotExist } // expect runtime error
}

(new Derived).foo
