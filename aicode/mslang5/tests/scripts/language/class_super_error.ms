class A {}

class B < A {
  method() {
    return super.nope();
  }
}

print B().method();
