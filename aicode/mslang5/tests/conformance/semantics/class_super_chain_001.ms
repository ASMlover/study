// @id: SEM-CLASS-SUPER-CHAIN-001
// @spec: semantics.class.inheritance
// @spec: semantics.class.super
// @expect: ok
// @stdout:
// ABC
class A {
  tag() {
    return "A";
  }
}

class B < A {
  tag() {
    return super.tag() + "B";
  }
}

class C < B {
  tag() {
    return super.tag() + "C";
  }
}

print C().tag();
