class A {
  method() {
    return "A"
  }
}

class B < A {
  method() {
    return "B"
  }

  chain() {
    return super.method() + this.method()
  }
}

print B().chain()
