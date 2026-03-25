class A {
  tag() {
    return "A"
  }
}

class B < A {
  tag() {
    return super.tag() + "B"
  }
}

class C < B {
  tag() {
    return super.tag() + "C"
  }
}

print C().tag()

